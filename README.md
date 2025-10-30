## ft_ping
ICMP Echo 기반 네트워크 진단 도구

### 프로젝트 개요
- 표준 `ping(8)`을 C99로 재구현한 실습용 프로젝트.
- 리눅스 RAW 소켓을 직접 열어 ICMP Echo Request/Reply 패킷을 송수신.
- poll 기반 이벤트 루프와 signalfd 를 이용해 신호와 소켓을 비동기 처리.

### ICMP Echo 동작 개요
- 송신: ICMP 헤더 + 전송 시각(`struct timespec`) + 패딩(`'a'`)으로 64바이트 패킷을 구성한 뒤 `sendto`로 전송.
- 수신: `recvfrom`으로 수신한 패킷의 IPv4/ICMP 헤더와 페이로드를 파싱, RTT 를 계산.
- 검증: 체크섬, 식별자/시퀀스, 페이로드 무결성을 확인해 가짜 응답과 손상 패킷을 배제.
- 통계: 전송/수신/에러 카운트와 min/avg/max/mdev RTT를 추적해 마지막에 요약 출력.

### 모듈 구성
- `srcs/init.c`: 목적지( `getaddrinfo` ), RAW 소켓, signalfd, 전송 타임스탬프 버퍼 초기화.
- `srcs/main.c`: 옵션 파싱(`-v`), 이벤트 루프, poll 로 소켓·신호 감시.
- `srcs/ping.c`: Echo Request 작성/전송, 응답 파싱, RTT 누적 로직.
- `srcs/validate.c`: IPv4/ICMP 헤더 필터링, 페이로드 일관성, 체크섬 검증.
- `srcs/ping_output.c`: 응답/에러 메시지 및 통계 포맷팅.
- `srcs/utils.c`: 체크섬, 시간 차 계산, 링버퍼 기반 시퀀스 관리, 리소스 정리.

공통 구조체와 매크로는 `includes/ping.h` 에, 오류 도우미는 `includes/error.h` 에 정의되어 있다.

### 빌드 & 실행
```bash
make            # cc -g 로 빌드, ./ft_ping 생성
sudo setcap cap_net_raw+ep ./ft_ping   # 일반 사용자 실행 시 RAW 소켓 권한 부여
./ft_ping 127.0.0.1
```

### 수동 테스트 시나리오
- 루프백: `./ft_ping 127.0.0.1` – 기본 RTT 계산과 통계 출력 확인.
- 외부 호스트: `./ft_ping example.com` – DNS 해석, TTL, RTT 변화를 관찰.
- 비도달 대상: `./ft_ping 203.0.113.1` – ICMP 오류 코드 출력 경로 검증.
- `-v` 옵션과 시스템 ping 결과를 비교해 출력 차이를 점검.

### 핵심 포인트
- signalfd 로 SIGINT를 파일 디스크립터로 다루어 깔끔한 종료 처리.
- RAW 소켓 송수신에서 endianness, 필드 정렬, 체크섬을 직접 관리.
- RTT 계산을 위해 송신 타임스탬프 링버퍼와 고해상도 `CLOCK_MONOTONIC` 사용.
- 최소/최대 RTT 초기화, mdev 계산 등 표준 ping이 제공하는 통계 기능을 모사.

### 향후 개선 아이디어
1. 추가 옵션 지원: `-c`, `-i`, `-t` 등 전송 제어와 TTL 조정.
2. IPv6 (`ICMPv6`) 확장 및 공통 인터페이스 분리.
3. 단위 테스트: 패킷 파서·검증 루틴을 위한 가짜 프레임워크 도입.

프로젝트 구조나 구현 상의 의문점이 있다면 언제든 문의 주세요.
