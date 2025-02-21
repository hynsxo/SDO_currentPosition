# SDO_currentposition

**포지션 제어로 5ms마다 모든 모터에게 명령을 보내는 상황 속에서, SDO communication으로 모터에게 신호를 보내고 손목 모터(Maxon)의 현재 위치 제일 최신 값 받아오기**

---

<img src="./images/maxon_epos4_application.png" alt="SDO communication">

**ㄴ Maxon모터 EPOS4 Application Notes SDO communication**

---

<img src="./images/senddata.png" alt="send images">
<img src="./images/sendSignal.png" alt="sendSignal">
<pre>
fun.appendToCSV_DATA("손목데이터", (float)maxonMotor->nodeId, maxonMotor->motorPosition, maxonMotor->motorTorque);
</pre>

**ㄴ 5ms (0.005) 단위로 데이터가 모터로 보내짐. (궤적 포지션 제어 명령)**

**ㄴ 아래 그래프에서, 빨강 그래프가 포지션 제어로 보내는 제어 명령**

---

<img src="./images/recvdata.png" alt="recv images">
<img src="./images/recvSignal.png" alt="recvSignal">
<pre>
fun.appendToCSV_DATA("q8손목", (float)maxonMotor->nodeId, maxonMotor->motorPosition, 0);
</pre>

**ㄴ SDO를 사용하여 받아오는 모터 현재 위치 값을 주기적으로 바로 받아 옴**

**ㄴ 파란 그래프는 SDO communication으로 받아오는 모터 현재 위치 값**

---


<img src="./images/currentValue.png" alt="currentValue images">
<img src="./images/타격감지시_현재위치.png" alt="타격감지시_현재위치">

**ㄴ CSP모드에서 CST모드로 전환되는 시점에 받아오는 현재 모터 위치 값 받아옴**
**ㄴ 타격감지 시 현재위치 값이, 그래프 상에서 정확하게 일치함 (=23.1601초)**
