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
        
        ![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/084c27ad-84d5-4651-b7bd-2323995f4552/Untitled.png)
        
        1) `Git push`를 `Webhook`이 탐지 
        2) `Jenkins` 내 `Docker`에서 현재 사용 중인 컨테이너 정지 및 삭제
        3) 새롭게 이미지 빌드
        4) 이미지 업로드
        5) 컨테이너 실행
        6) 위 과정을 완료한 이후 `Discord`로 알림
