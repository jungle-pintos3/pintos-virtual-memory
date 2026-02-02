# 🖥️ Pintos: Virtual Memory Implementation

KAIST Pintos 프레임워크 기반의 가상 메모리 관리 시스템 구현 프로젝트입니다.

## 📌 프로젝트 개요

이 프로젝트는 교육용 운영체제인 Pintos를 기반으로 **가상 메모리 관리 시스템**을 구현한 결과물입니다. 페이징, 페이지 폴트 처리, 스왑, 메모리 맵 파일 등 현대 운영체제의 핵심 메모리 관리 기법을 직접 구현했습니다.

> **Based on**: [KAIST Pintos](https://casys-kaist.github.io/pintos-kaist/) - 64-bit x86 아키텍처 지원

## 🎯 주요 구현 기능

### 1. **Memory Management**
- 페이지 테이블 관리 (Page Table Management)
- 보조 페이지 테이블 (Supplemental Page Table)
- 프레임 테이블 관리 (Frame Table)

### 2. **Demand Paging**
- Lazy Loading을 통한 효율적인 메모리 사용
- 페이지 폴트 핸들링 (Page Fault Handler)
- 스택 성장 (Stack Growth) 지원

### 3. **Swap Management**
- 스왑 인/아웃 메커니즘
- 페이지 교체 알고리즘 (Page Replacement Algorithm)
- 비트맵 기반 스왑 슬롯 관리

### 4. **Memory-Mapped Files**
- `mmap()` / `munmap()` 시스템 콜 구현
- 파일과 메모리 간 매핑
- 변경사항 동기화

## 🛠️ 기술 스택

- **Language**: C
- **Architecture**: x86-64
- **Emulator**: QEMU
- **Development**: Docker
- **OS**: Ubuntu 22.04 (Container)

## 📂 프로젝트 구조

```
pintos/
├── vm/              # Virtual Memory 구현
│   ├── vm.c         # 가상 메모리 핵심 로직
│   ├── anon.c       # Anonymous 페이지 관리
│   ├── file.c       # 파일 백업 페이지
│   └── uninit.c     # 초기화되지 않은 페이지
├── userprog/        # 유저 프로그램 지원
├── threads/         # 스레드 및 동기화
└── lib/             # 라이브러리 및 유틸리티
```

## 🚀 빌드 및 실행

### 빌드
```bash
cd pintos/vm
make
```

### 테스트 실행
```bash
# 전체 테스트
make check

# 특정 테스트
make tests/vm/page-linear.result
```

## ✅ 테스트 결과

가상 메모리 관련 모든 테스트 케이스 통과:
- Page allocation/deallocation
- Stack growth
- Memory-mapped files
- Swap in/out operations

## 💡 핵심 학습 내��

- **페이징 시스템**: 가상 주소와 물리 주소 변환 과정 이해
- **메모리 효율성**: Lazy loading을 통한 리소스 최적화
- **동시성 제어**: 멀티스레드 환경에서의 메모리 동기화
- **시스템 프로그래밍**: 저수준 메모리 관리 및 하드웨어 인터페이스

## 🔗 참고 자료

- [Pintos 공식 문서](https://casys-kaist.github.io/pintos-kaist/)
- [KAIST CS330 - Operating Systems](https://casys-kaist.github.io/)

## 📝 License

This project is based on the Stanford Pintos project, modified by KAIST.

---

**Note**: 이 프로젝트는 운영체제 학습을 목적으로 작성되었습니다.
