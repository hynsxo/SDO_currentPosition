# SDO_currentposition

**포지션 제어로 5ms마다 모든 모터에게 명령을 보내는 상황 속에서, SDO communication으로 모터에게 신호를 보내고 손목 모터(Maxon)의 현재 위치 제일 최신 값 받아오기**

---

<img src="./images/maxon_epos4_application.png" alt="send images">

**ㄴ Maxon모터 EPOS4 Application Notes SDO communication**

---

<img src="./images/senddata.png" alt="send images">
<pre>
fun.appendToCSV_DATA("손목데이터", (float)maxonMotor->nodeId, maxonMotor->motorPosition, maxonMotor->motorTorque);
</pre>

**ㄴ 5ms (0.005) 단위로 데이터가 모터로 보내짐. (궤적 포지션 제어 명령)**

---

<img src="./images/recvdata.png" alt="recv images">
<pre>
fun.appendToCSV_DATA("q8손목", (float)maxonMotor->nodeId, maxonMotor->motorPosition, 0);
</pre>

**ㄴ SDO를 사용하여 받아오는 모터 현재 위치 값을 주기적으로 바로 받아 옴.**


---
