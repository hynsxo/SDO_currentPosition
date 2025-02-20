if (std::shared_ptr<MaxonMotor> maxonMotor = std::dynamic_pointer_cast<MaxonMotor>(motor))
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