# 📚 프로젝트
## 1️⃣ ChatGPT를 이용한 면접 준비 사이트 ‘인터뷰연구소’ 제작

### 📆 기간 : 2023. 06. ~ 2023. 08.

### 👩‍💻 참여 인원 : 4명

### 📔 설명
- chatGPT를 이용하여 면접 질문을 받고 피드백까지 받는 면접 준비 사이트

### 🔧 역할 / 기여도
- 프로젝트 팀장
- 주제 선정 - 기여도 : 90%
- ChatGPT를 통해 면접 질문 받기 및 피드백 기능 구현 - 기여도 : 100%
- 게시판 글 및 댓글 조회/작성/수정/삭제 기능 구현 - 기여도 : 100%
- Jenkins를 이용하여 자동화 배포 시스템 구축 - 기여도 : 100%
- 전반적인 회의록 작성 및 문서 정리 - 기여도 : 70%
- AWS Route53를 이용하여 https 적용 - 기여도 : 50%
- 로그인 API 구현 - 기여도 : 30%
- Swagger 작성 - 기여도 : 30%

### 📌 느낀 점
- 다른 팀 프로젝트와는 새로운 점
    - 이전에는 어느 정도 정해진 틀에서 원래 알던 사람들과 하는 프로젝트가 많았는데, 이번에는 하고 싶은 주제를 가지고 뜻이 맞는 팀원을 구해 프로젝트를 진행하였다. 다른 때와는 달리 서로의 능력이나 성격을 잘 모르는 상태에서 하다 보니 그에 따른 문제도 있었지만 나중에는 팀원들과의 소통으로 잘 해결해 나간 것 같아 뿌듯했다.
    - 이전 프로젝트와는 다르게 백엔드의 많은 부분을 담당해볼 수 있어서 좋았다. 지난 번에는 기능 개발에만 집중했다면 이번에는 CI/CD부터 도메인 적용까지 해볼 수 있어서 프로젝트 할 때 바라보는 시야가 좀 더 넓어진 것 같다.
    - 몇 번의 프로젝트를 하면서 해결했던 오류나 기능 개발 했을 때 시도했던 방법 등을 같은 걸 기록해두면 나중에 도움이 된다는 걸 깨달았다. 그래서 이번 프로젝트 시작부터 했던 것들을 개인 블로그에 기록을 해두었는데 실제로 프로젝트 진행하면서 정말 많은 도움이 되었다. Jenkins에 Docker 설정하는 방법을 정리해서 적어두었는데, 이후에 지우고 다시 초기 설정부터 해줘야 할 때 적어둔 것을 보고 하니 시간이 훨씬 많이 절약되었다.
- SQL
    - 여러 자격증을 공부하며 이론적으로 SQL에 대해서 공부했지만 실제로 사용해본 적은 없었기에 이번 기회에 사용해보고 싶어 선택했다. 공부를 어느 정도 했기에 쉽게 사용할 수 있을 거라 생각했지만 생각했던 것보다 훨씬 어려웠다. 그 중에서도 `Forgien Key` 설정이 가장 어려웠다.
        - Schema 설정 시 아래와 같은 방법으로 설정
            - 이 관계를 이해하는 것도 시간이 걸렸지만, Schema 부분을 새로 만들었을 때 테이블을 재 생성 해 주어야 한다는 사실을 모르고 계속 안 된다고만 생각했었다.
            테이블 재 생성을 하고 나면 정상적으로 DB에 `Forgien Key`로 설정 된 것을 확인했다.
            
            ```jsx
            // User - ChatGPTList
            db.User.hasMany(db.ChatGPTList, {
              foreignKey: 'userId',
              sourceKey: 'id',
              onDelete: 'cascade'
            });
            db.ChatGPTList.belongsTo(db.User, {
              foreignKey: 'userId',
              targetKey: 'id',
              onDelete: 'cascade'
            });
            ```         
- 다양한 Error 해결
    - `openai` 401 Error
        - 잘 사용되던 ChatGPT API가 갑자기 401 Error를 띄워 문제점을 찾아보니 Git에  `OPENAI_API_KEY`가 노출되어 발생하는 문제였다. 이후 `Jenkins` 환경 변수로 설정해주니 해결되었다.
    - `CORS` Error
        - `Response Header` 설정을 변경해주니 오류 해결
    
    ```jsx
    app.use(
      '/api',
      (req, res, next) => {
        const allowedOrigins = [
          'http://localhost:5173',
          'https://interviewlab.site'
        ];
        const origin = req.headers.origin;
        if (allowedOrigins.indexOf(origin) !== -1) {
          res.header('Access-Control-Allow-Origin', origin);
        } else {
          res.header('Access-Contorl-Allow-Origin', 'http://localhost:5173');
        }
        res.header('Access-Control-Allow-Credentials', 'true');
        res.header(
          'Access-Control-Allow-Headers',
          'Origin, X-Api-Key, X-Requested-With, Content-Type, Accept, Authorization'
        );
        res.header('Access-Control-Allow-Methods', 'GET, POST, PUT, PATCH');
        res.header('Access-Control-Expose-Headers', 'Authorization');
        next();
      },
      indexRouter
    );
    ```  
- CI/CD
    - CI/CD를 직접 해보면서 배포를 좀 더 편하게 관리할 수 있게 되었다. 프로젝트 초기에 완성하고 싶었지만 자꾸 실패해서 뒤로 밀려나게 되었다. 그렇지만 중간 중간 포기하지 않고 끝까지 해 내어서 완성할 수 있었다.
        
        ![image](https://lh3.googleusercontent.com/fife/AK0iWDzP1Jbbvr8Jtk_u_Z3uyH3ht32NYaD5fyJ6Kcuv6uapVWw9ivCNoBTOaJb0zTRK8uINERgOskao57pt8OETRfRfKK88GPkLpE9TTMFbmUr2ya9_w1IgZhRNRTErcg1oZMhTyA1BakF-2tIMgIQZPLwfSPrWOM7e4YGe4fxDwD2IQbAwKhDytyujW941Ks3PNBLCqoXF6dZa_GPSYWfQn49ffVp50vVck72kko8ijTIltWlo6H5yx4aMS6V14ppVRP_4-NKeYb7zEsRTCCQuGHRjKbrDZv0Wfn1FYp3USQufu9js3VA67SZM7ZDRxBCh69ZiJ5jNsm0w_G7ip6zBs-3PD50zKcHWB48VrYSi3hhJf9eyf2q3SLTXw2eHvYznoTdaaFMNstvI_1tIPY-VVaMiR8Sqk7flnveFBS_h2d6s46nSrg7DeFtvt-KSuMaGIwD-as_0f0mQGb8U6tXXZBGogyu4PkCUhn29Ufq5eVg4U8bUvzncobzq9nPUl-4eZHuPS_vTBz-nSSksk3yq0bCDrvoe_lUTcBvpEKWeJbT95ncvGYTsbrH8lPYvF36-XurbGjSEWIqYGJVJwlS2RNEhg49iwAktbUf5l5GfTk5glxrksw81ddnMapxQ8FOx-mHyBkAEed-PFYF0j5pfGnD1MTvBZewAA1OgJBX5ROv0ZmFAphowkvco8Act8L9SW9VnbEiWKil3-QDC7S1CAaK01-gMrgesxkDktLyQJuGWETIMoecSBQ54ACF5Ms2qwpHkpWDM19Qo-gyk2gWTvkTBZgBvL5MC4k6NsRmKWkkfGHXOLGYPdTYAxpDIBS8EYL_FSTprnAJ96yYyscs6Z6Vh9KBMSJG9TqJBZvkxfmJWFx-0xeVAyL9KlYu2893fXqy07-7tritPfMS3BYT3KWyUXzqfj1s3b6I17QlvKtEtSu5XKA8OVuOKEjnGEPASTFvVB3sKJAbF4IB3R4QO7TNN0K9KgQWKHnUF9EDrb1f0KzHafr0gd-GWhneeUS62uubnukXMCTCioj-grYoomOJvqup8vReLrHpRHGRCF2-HKUQ_T7i907oZOKPLILpt3I8fWImSU_dwI3rw910gdInhT4RQuOCtnUCkcqxrpR_Y1nink9PXNAgSFU37_sX6tzp3glvPTDwAESv2M4hkh_EPSiQOU9cwAbU13JeLGcz1Dka6miJt3ri3N4-gA4bCYBpn1OMEr3p4hfL3LtupppkCCZ-3KwlhGiocQjFvjaJ8SxtMprcafqyalEnc1T9oS5Z7pqI2EUQe1sgWCvP0yO9BiobJjuRbupV8oHUmXzA40csJVk8NYJ3L9qazBHr3yPoLoglmXKptRUAFB3PeMlMUoLnB2QlgJw0PR_C6k0C_W10A96tsDtY2hb7T2kFcJ4dnizltbHBlUdN2uxCTz1Qydn4q3hfXJqX6_09xvpRzZ_118XG-OyvGYynwP1gtFpxOyI-AyljOLbsJk7sobX1tSqC9nASv7GyoydXv9XoINXn-6lf43AGtpZxBEzcC9GTbcQcNBncpbEqB4RFrXzyW2HXGCCvuobEF6sZdRJEmL1HKNxlpaZf1JASgZauhMa1JqEY-3MljY1IlmeqZIFA2FMir_hfIEitVlTAgWCWLlwwufpqpnvypjFsYdWYuvomxR_v_xuzJlJHQ_aVuvKvryw=w1920-h878)
        
        1) `Git push`를 `Webhook`이 탐지 
        2) `Jenkins` 내 `Docker`에서 현재 사용 중인 컨테이너 정지 및 삭제
        3) 새롭게 이미지 빌드
        4) 이미지 업로드
        5) 컨테이너 실행
        6) 위 과정을 완료한 이후 `Discord`로 알림


## 2️⃣ 와인 판매 사이트 ‘Bottle Shop’ 제작

### 📆 기간 : 2023. 02. ~ 2023. 03.

### 👩‍💻 참여 인원 : 4명

### 📹 시현 영상

**[https://youtu.be/qjYt-25Vd5s](https://youtu.be/qjYt-25Vd5s)**

### 📔 설명

- 회원가입 / 로그인 기능을 사용하여 사용자 식별 및 인증
- 사용자 계정
    - 메뉴 상세 정보 확인
    - 장바구니 확인 및 배송 정보 저장 가능
    - 사용자 정보 확인·수정·삭제 가능
- 관리자 계정
    - 상품 정보 추가·조회·수정·삭제
    - 사용자 정보 추가·조회·수정·삭제

### 🔧 역할 / 기여도

- DB 구조 설계 – 기여도 : 50%
- 업무 흐름도 및 시퀀스 다이어그램 작성 – 기여도 : 50%
- 회원가입 기능 – 기여도 : 100%
- 로그인 기능 – 기여도 : 100%
- 주문 조회 기능 - 기여도 : 10%
- API 제작 - 기여도 : 50%

### 📌 느낀 점

- 커뮤니케이션
    - 팀원 모두가 Git을 처음 사용하다보니, 가끔 **Merge**로 인하여 `Merge conflict` 오류가 자주 발생 하였다 처음에는 당황해서 Local에서 직접 수정하여 해결했지만, 팀원들과 함께하며, 방법을 찾아 해결하였던 기억이 인상 깊었다.
    - 매일 정해진 시간에 **스크럼**을 가졌는데, 굉장히 도움이 많이 되었다. 스크럼을 통해 오늘 하루 어떤 부분을 할 것이다라는 **목표**도 세울 수 있고, 전체적인 **프로젝트의 진행 상황**도 확인해볼 수 있어서 좋았다. 또한 어제에 비해 진행이 느린 팀원이 있다면 빠르게 도움을 줄 수 있어서 이 부분도 좋았다.
- 개발 하면서
    - 눈에 보이는 사이트를 **직접** 만들어냈다는 것이 가장 뿌듯한 점이었다. 실제로 만든 기능이 동작할 때 신기했고 그저 일상적으로 사용하던 다른 사이트들을 이용할 때에도 이걸 구현하기 위해서는 어떻게 했을까라는 고민을 하게 되었다.
    - **API 문서**를 미리 작성해놓지 않고 기능을 구현하면서 작성하다보니 Front-End에서 사용할 때 자꾸 오류가 발생하였다. 처음에는 API 문서가 중요하지 않다고 생각했지만 막상 프로젝트가 끝날 때쯤에는 제일 중요한 것이구나를 깨달았다.
    - API를 제작했지만 사이트에 적용하지 못한 것, 로그인 기능 관련하여 기능 고도화를 하려고 하였으나 하지 못한 것이 아쉬웠다. 술 관련 사이트이기 때문에 **성인인지 확인**하는 기능과 **이메일 인증** 기능을 개선해보고 싶다.


## 3️⃣ 블록체인을 이용한 질소산화물의 조작 방지 시스템

### 📆 기간 : 2019. 06. ~ 2019. 11.

### 👩‍💻 참여 인원 : 6명

### 📔 설명

- 먼지 센서로부터 질소산화물(미세먼지) 정보를 Server에 저장
- 정보의 조작 방지를 위해 다른 Rasp에서 올바른 측정값인지 검증
- 다른 Rasp들의 데이터 값 확인

![image](https://user-images.githubusercontent.com/121041665/232259111-b3e33ad6-b878-49b0-b943-2fbb2edc2016.png)

### 🔧 역할 / 기여도

- 시스템 및 프로토콜 구상 - 기여도 : 30%
- Rasp-Server 사이의 Serial 통신 구현 - 기여도 : 100%
- Server에서 여러 개의 Rasp 요청을 Thread로 처리 - 기여도 : 100%
- Server에서 Rasp에게 받은 정보 저장·검증 - 기여도 : 50%

### 📌 느낀 점

- 첫 팀 프로젝트를 해보며
    - 동기가 아닌 선배와 함께 하고 여러 명과 함께한 프로젝트가 처음이라 소통에 익숙하지 않은 것이 아쉬웠다. 특히 프로토콜을 **구상 초기에 확실하게 정해놓지 않아** 개발을 진행하면서 코드를 합칠 때 어려움을 겪었다.
    - 혼자 개발을 진행할 때보다 팀원과 함께 했을 때 **동기부여**도 되고 개발 하는 내내 즐거웠다.
- 프로젝트를 진행하기 위해서 **Blockchain**에 관해 공부했다. **비트코인, 블록체인과 금융의 혁신**이라는 책을 통해 개념을 공부하여 직접 적용해 개발해보니 그저 이론으로 공부할 때보다 **훨씬 이해**가 잘 되었다.
- 여러 개의 Rasp에서 Server와의 통신을 연결하기 위해 동시에 요청을 처리해야 하는 문제가 발생하였는데, 이때 **Thread**를 만들어 각각의 요청을 처리하였다. 이전까지는 1대1 통신에 대해서만 고민했다면 이번을 계기로 1대N, N대N 통신에도 공부를 해보아야겠다고 다짐했다.

### ✨ 성과

- ‘2020 블록체인 아이디어 공모전’에서 한국인터넷진흥원 원장상 수상


## 4️⃣ IoT 보안 인증 제도 기반 홈 IoT 기기 애플리케이션의 취약점 분석 및 대응책 제시

### 📆 기간 : 2021. 01. ~ 2021. 06.

### 👩‍💻 참여 인원 : 4명

### 📔 설명

- 홈 IoT 기기 애플리케이션에 대해 NOX 프로그램으로 추출한 APK를 MITMPROXY, SUPER 툴을 이용해 각각의 취약성을 분석하여 발생 가능한 위협을 식별
- KISA의 ‘IoT 보안 시험·인증 기준 해설서’에 따른 인증 기준 부합 여부를 평가 및 적절한 보안 대응책 제시

### 🔧 역할 / 기여도

- 프로젝트 팀장
- 주제 선정 및 IoT 기기 선정 - 기여도 : 30%
- 네트워크 분석 툴인 MITMPROXY를 이용하여 MITM 공격 수행 - 기여도 : 50%
- 홈 IoT 기기 애플리케이션의 APK 파일 속 JAVA 파일 분석 - 기여도 : 100%
- KISA의 ‘IoT 보안 시험·인증 기준 해설서’에 따른 인증 기준 부합 여부를 평가 및 적절한 보안 대응책 제시 - 기여도 : 50%
- 논문 작성 - 기여도 : 30%

### 📌 느낀 점

- 블록체인 관련 프로젝트를 진행하면서 **아쉬웠던 점을 보완하여 진행**해보고자 팀장을 자원하였다. 특히 주어진 시간 안에 해야 했기 때문에 **시간 관리**를 철저하게 하기로 마음을 먹었다. 또 소통을 중시하여 적어도 일주일에 한 번은 꼭 세운 **계획에 대해 검토**하는 시간을 가졌다. 이렇게 진행하니 훨씬 효율적으로 프로젝트를 진행할 수 있었다.
- 처음 계획은 IoT 펌웨어의 취약점 분석을 목표로 하였다. 하지만 여러 분석 방법으로 시도해보았지만 코드를 볼 수 없었다. 이미 이에 대해 시간을 많이 쏟았기 때문에 남은 시간이 얼마 남지 않아 고민을 많이 했다. 그래서 펌웨어 분석을 포기하고 소프트웨어 분석으로 바꾸었다. 처음 계획대로 진행되지는 못하였지만 그래도 결과물을 만들 수 있어서 뿌듯했다.
- 홈 IoT 기기 애플리케이션의 **APK 파일 속 JAVA 파일을 분석**하였다. 남이 만들어놓은 코드를 보고 어떤 기능을 하는지 분석하는 것을 처음 해보았는데, 생각보다 아주 어려웠다. 파일의 개수도 하나가 아니기도 하고, 어떤 한 기능이 어떻게 동작하는지 찾기 위해서는 **복잡한 과정**을 거쳐야 했다. 또, 정답이 있는 것도 아니다 보니 답답할 때도 많았지만 코드를 이해하고 나니 문제를 해결했다는 **성취감**을 얻었다.

### ✨ 성과

- 논문 ‘IoT 보안 인증 제도 기반 홈 IoT 기기 애플리케이션의 취약점 분석 및 대응책 제시’ 작성
- ‘2021년도 한국정보보호학회 하계학술대회’에서 한국전자통신연구원 원장상 수상


## 5️⃣ APT 공격 - EDR 탐지 시스템

### 📆 기간 : 2021. 08. ~ 2021. 11.

### 👩‍💻 참여 인원 : 10명

### 📹 시현 영상

[https://youtu.be/-a5s87kVHDY](https://youtu.be/-a5s87kVHDY)

### 📔 설명

- APT 모사 시스템을 구성하여 피해자에게 악성코드를 유포한 다음 중요 데이터를 탈취하는 시나리오를 구성 후 공격 실행
- 피해자 엔드포인트에서 특정 이벤트에 대한 로그를 수집 후, 분석하여 악성행위로 판단되면 관리자에게 경고 알림 전송 및 대응
- 시스템 내 주요 서버들의 로그를 수집하여 실시간 모니터링
- 대시보드를 통한 모니터링 결과 시각화

### 🔧 역할 / 기여도

- 프로젝트 팀장, PM
- 발표 자료 및 문서 정리 - 기여도 : 60%
- APT 시나리오 제작 - 기여도 : 40%
- DNS 서버 · 메일 서버 구축 - 기여도 : 50%
- 웹 서버 · 파일 서버 구축 - 기여도 : 100%

### 📌 느낀 점

- 이전 프로젝트를 했을 때 팀장을 하면서 느낀 **보람**과 일을 잘 해낼 수 있다는 **자신감**으로 이번에도 자원하였다. 하지만 팀의 인원이 많을 때의 팀장 일은 **더더욱** 힘들다는 것을 깨달았다. 또 팀원 대부분이 내향적이라 힘든 점이나 어려운 점에 대해 쉽게 이야기하지 않는 점도 힘들었다. 그럴수록 한 명 한 명 개인적으로 이야기를 나누며 팀원들과 **친해지기 위해 노력**하였다. 그 결과 **그만두는** 팀원 없이 끝까지 마칠 수 있었다.
- DNS 서버와 메일 서버의 경우, `bind9`와 `sendmail`, `dovecot` 패키지를 이용하여 구축하였다. 서버를 구축하면서 제일 힘들었던 때는 PC 2대로 진행하였을 때 **bind 에러**가 자주 발생할 때였다. 어떤 것을 **DNS 서버 주소**로 정해야 할지 **방화벽 설정**은 어떻게 해야 하는지 등 다양한 고민을 했었다. 직접 만든 메일주소로 메일을 주고받았을 때 들었던 감정은 힘든 작업이 끝났다는 해방감 반, 진짜 메일을 주고받았다는 것에 대한 신기함 반이었다.
- 웹 서버와 파일 서버는 구축은 했지만 실제 시나리오에는 사용하지 못했다. 웹 서버는 `apache`를, 파일 서버는 `FileZila`를 이용하였다. 사용하지 않았기에 기본적인 기능만 작동하였지만 직접 구현해보는 경험해볼 수 있어서 좋았다.
    
 
 
# 💻 인턴

## 1️⃣ 시큐리티플랫폼 (2021-06-22~2021-08-17)

### 📕 수행 내용

- CAVP 검증 Tools 개발
    - 시큐리티플랫폼에서 개발한 암호 모듈의 암호 알고리즘 구현 정확성을 검증하는 CAVP Tools 개발
- TCP/IP Echo Server & Client 개발
    - Server-Client 소켓 통신 프로그래밍

### 📘 느낀 점

- 실무 경험 및 실무자와의 소통 능력 향상
    - 실제 회사에서 두 달간 실무자들과 함께 일하는 경험을 해보니 확실히 학교에서 공부할 때와는 다른 느낌이었다. 업무를 보고할 때나 Git을 통해 업무를 하는 등 그저 공부만으로는 할 수 없는 경험을 해본 것이라 좋았다.
- 소스코드 분석 능력 향상
    - 실제 개발 중인 알고리즘을 보고 CAVP 검증 Tool을 개발해야 했기 때문에 제일 처음 했던 것은 소스코드 분석이었다. 또, 새로 코드를 작성할 때도 다른 사람이 봐도 이해가 잘되도록 주석을 달아주거나 이전 코드 변수나 양식을 맞춰서 작성하는 등 함께 작업할 때의 매너를 배울 수 있었다.
    
    
    
