for (auto &motor_pair : motors)
{
    auto &motor = motor_pair.second;

    if (!motor->isConected)
    {
        motor->socket = socket_fd;
        if (std::shared_ptr<MaxonMotor> maxonMotor = std::dynamic_pointer_cast<MaxonMotor>(motor))
        {
            maxoncmd.getCheck(*maxonMotor, &frame);
            txFrame(motor, frame);
        }

        usleep(50000);
    }
}