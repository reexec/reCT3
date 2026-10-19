# CT3 XBE 구조와 설계 (한국어)

원본 XBE 분석으로 확인한 구조를 누적한다. 상세 근거와 확인 상태는 [`docs/analysis/`](analysis/README.md)에 있고, 여기에는 설계에 영향을 주는 결론만 모은다. 영어판: [XBE_DESIGN.en.md](XBE_DESIGN.en.md).

## 1. 대상 빌드

* 1차 타깃은 일본판 `default.xbe`다. 신원(title id, 인증서 시각, 이미지 시각, MD5)은 XBE가 준비되면 `scripts/recomp/register_xbe.py`로 등록한다. → [ct3-xbe-overview §1](analysis/ct3-xbe-overview.md)
* 다른 지역판이 발견되면 식별만 하고, 주소는 공유하지 않는 것으로 본다(JSRF에서 두 지역판의 `.text`가 달랐던 선례, **추정**).

## 2. 메모리 배치

XBE를 읽으면 채운다(`rexbox_xbe_info`).

## 3. HLE 설계에 주는 결론

* XDK 버전이 확인되면 rexbox의 D3D8 HLE(XDK 4134 기준 측정값)와의 차이를 여기에 적는다.

## 4. 시작 순서와 커널 구조

* XAPI 시작 코드와 메인 스레드 생성 순서는 JSRF와 같은 XAPI 라이브러리면 같을 것이다(**추정**). 부팅 frontier 분석에서 확인한다.

---

(영어 번역은 [XBE_DESIGN.en.md](XBE_DESIGN.en.md)에 있다.)
