void DrumRobot::ReadProcess(int periodMicroSec)
{
    std::shared_ptr<MaxonMotor> maxonMotor = std::dynamic_pointer_cast<MaxonMotor>(motors["L_wrist"]);
    std::shared_ptr<GenericMotor> motor = motors["L_wrist"];

    struct can_frame frame;

    auto currentTime = chrono::system_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::microseconds>(currentTime - ReadStandard);
    
    switch (state.read.load())
    {
    case ReadSub::TimeCheck:
        if (elapsed_time.count() >= periodMicroSec)
        {
            state.read = ReadSub::ReadCANFrame; // 주기가 되면 ReadCANFrame 상태로 진입
            ReadStandard = currentTime;         // 현재 시간으로 시간 객체 초기화

            maxoncmd.getActualPos(*maxonMotor, &frame); //Maxon모터에게 1ms마다 신호
            canManager.txFrame(motor, frame); // CAN을 통해 보내줌
        }
        break;
    case ReadSub::ReadCANFrame:
        canManager.readFramesFromAllSockets(); // CAN frame 읽기
        state.read = ReadSub::UpdateMotorInfo; // 다음 상태로 전환
        break;
    case ReadSub::UpdateMotorInfo:
    {
        
        // if ((!settingInitPos) || state.main == Main::Test) // test 모드에서 에러 검출 안함
        // {
        //     canManager.distributeFramesToMotors(false);
        // }
        if (!settingInitPos)    // test 모드에서 에러 검출함
        {
            canManager.distributeFramesToMotors(false);
        }
        else
        {
            bool isSafe = canManager.distributeFramesToMotors(true);
            if (!isSafe)
            {
                state.main = Main::Error;
            }
        }

        state.read = ReadSub::TimeCheck;
        break;
    }
    }
}
bool CanManager::distributeFramesToMotors(bool setlimit)
{
    for (auto &motor_pair : motors)
    {
        auto &motor = motor_pair.second;

        if (std::shared_ptr<TMotor> tMotor = std::dynamic_pointer_cast<TMotor>(motor))
        {
            // TMotor 처리
            for (auto &frame : tempFrames[motor->socket])
            {
                if ((frame.can_id & 0xFF) == tMotor->nodeId)
                {
                    std::tuple<int, float, float, float, int8_t, int8_t> parsedData = tservocmd.motor_receive(&frame);
                    
                    tMotor->motorPosition = std::get<1>(parsedData);
                    tMotor->motorVelocity = std::get<2>(parsedData);
                    tMotor->motorCurrent = std::get<3>(parsedData);

                    // tMotor->jointAngle = std::get<1>(parsedData) * tMotor->cwDir * tMotor->timingBeltRatio + tMotor->initialJointAngle;
                    tMotor->jointAngle = tMotor->motorPositionToJointAngle(std::get<1>(parsedData));
                    
                    // std::cout << tMotor->jointAngle << std::endl;
                    tMotor->recieveBuffer.push(frame);

                    fun.appendToCSV_DATA(fun.file_name, (float)tMotor->nodeId, tMotor->motorPosition, tMotor->motorCurrent);
                    
                    if (setlimit)
                    {
                        bool isSafe = safetyCheck_T(motor);
                        if (!isSafe)
                        {
                            return false;
                        }
                    }
                }
            }
        }
        else if (std::shared_ptr<MaxonMotor> maxonMotor = std::dynamic_pointer_cast<MaxonMotor>(motor))
        {
            // MaxonMotor 처리
            for (auto &frame : tempFrames[motor->socket])
            {
                // Maxon모터에서 1ms마다 SDO 응답 확인
                if (frame.can_id == (0x580 + maxonMotor->nodeId)) {
                    if (frame.data[1] == 0x64 && frame.data[2] == 0x60 && frame.data[3] == 0x00) { 
                        int32_t pos_enc = frame.data[4] | (frame.data[5] << 8) | (frame.data[6] << 16) | (frame.data[7] << 24);

                        float pos_degrees = (static_cast<float>(pos_enc) * 360.0f) / (35.0f * 4096.0f);
                        float pos_radians = pos_degrees * (M_PI / 180.0f);  
                        maxonMotor->motorPosition = pos_radians;

                        fun.appendToCSV_DATA("q8손목", (float)maxonMotor->nodeId, maxonMotor->motorPosition, 0);
                        current_Position = maxonMotor->motorPosition; // 현재 위치 값 해당 변수에 덮어쓰기
                    }
                }

                

                if (frame.can_id == maxonMotor->rxPdoIds[0])
                {
                    // getCheck(*maxonMotor ,&frame);
                    std::tuple<int, float, float, unsigned char> parsedData = maxoncmd.parseRecieveCommand(*maxonMotor, &frame);
                    
                    maxonMotor->motorPosition = std::get<1>(parsedData);
                    maxonMotor->motorTorque = std::get<2>(parsedData);
                    maxonMotor->statusBit = std::get<3>(parsedData);
                    maxonMotor->positionValues.push(std::get<1>(parsedData));
                    if ((maxonMotor->positionValues).size() >= maxonMotor->maxIndex)
                    {
                        maxonMotor->positionValues.pop();
                    }

                    // maxonMotor->jointAngle = std::get<1>(parsedData) * maxonMotor->cwDir + maxonMotor->initialJointAngle;
                    maxonMotor->jointAngle = maxonMotor->motorPositionToJointAngle(std::get<1>(parsedData));
                    maxonMotor->recieveBuffer.push(frame);
                    
                    fun.appendToCSV_DATA(fun.file_name, (float)maxonMotor->nodeId, maxonMotor->motorPosition, maxonMotor->motorTorque);

                    fun.appendToCSV_DATA("손목데이터", (float)maxonMotor->nodeId, maxonMotor->motorPosition, maxonMotor->motorTorque);


                    if (setlimit)
                    {
                        bool isSafe = safetyCheck_M(motor);
                        if (!isSafe)
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }
    tempFrames.clear(); // 프레임 분배 후 임시 배열 비우기

    return true;
}
void MaxonCommandParser::getActualPos(MaxonMotor &motor, struct can_frame *frame)
{
    frame->can_id = motor.canSendId;  // SDO Write 요청
    frame->can_dlc = 8;
    frame->data[0] = 0x40;  // SDO Write 명령 (4바이트 데이터)
    frame->data[1] = 0x64;  
    frame->data[2] = 0x60;  
    frame->data[3] = 0x00;  
    frame->data[4] = 0x00;  
    frame->data[5] = 0x00;  
    frame->data[6] = 0x00;  
    frame->data[7] = 0x00;  
}
bool CanManager::txFrame(std::shared_ptr<GenericMotor> &motor, struct can_frame &frame)
{
    if (write(motor->socket, &frame, sizeof(frame)) != sizeof(frame))
    {
        perror("CAN write error");
        return false;
    }
    return true;
}