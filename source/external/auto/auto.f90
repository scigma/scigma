!     ------- ----
      MODULE AUTO
      
      USE ISO_C_BINDING
      USE AUTOMPI
      USE IO
      USE SUPPORT, ONLY: NAMEIDX, AUTOSTOP
      USE AUTO_TYPES,ONLY: AUTOCONTEXT, AUTOPARAMETERS, NPARX
!$    USE OMP_LIB
      USE COMPAT

      CONTAINS

      SUBROUTINE AUTENTRY(ID, CFILENO) BIND (C,name="auto_entry")

      USE ISO_C_BINDING, ONLY: C_CHAR

      IMPLICIT NONE

      INTEGER ID
      CHARACTER (KIND=C_CHAR, LEN=1), DIMENSION(9), INTENT(IN) :: CFILENO
      CHARACTER (8) :: FILENO
      
      LOGICAL EOF,KEYS
! Local
      DOUBLE PRECISION TIME0,TIME1,TOTTIM
      INTEGER I,LINE,ios,UNITC
      INTEGER,ALLOCATABLE :: IICU(:)
      LOGICAL FIRST

      TYPE(AUTOCONTEXT) AC

! Get Fortran string filename from character array in CFILE

      DO I=1,8
         FILENO(I:I)=CFILENO(I)
      END DO

! Initialization :
! ID and i/o units
      AC%ID=ID
      AC%CUNIT=(ID+1)*4+3
      AC%SUNIT=(ID+1)*4+4
      AC%BUNIT=(ID+1)*4+5
      AC%DUNIT=(ID+1)*4+6
      
      CALL MPIINI()
      IF(MPIIAM()/=0)THEN 
         CALL MPIWORKER(AC) ! not called with nompi.f90 
         ! never returns
      ENDIF

       FIRST=.TRUE.
       UNITC=AC%CUNIT
       OPEN(UNITC,FILE='c.'//FILENO,STATUS='old',ACCESS='sequential',IOSTAT=ios)
       IF(ios/=0)THEN
          UNITC=5
       ENDIF


       KEYS=.FALSE.
       LINE=0
       DO
          IF(MPIKWT()>1)THEN
             CALL MPITIM(TIME0) ! not called with nompi.f90
          ELSE
             TIME0=AUTIM()
!$           TIME0=omp_get_wtime()
          ENDIF
          CALL INIT(AC,UNITC,EOF,KEYS,LINE)
          IF(EOF)EXIT
          CALL FINDLB_OR_STOP(AC,UNITC)
          CALL MPIIAP(AC%AP)
          ALLOCATE(IICU(SIZE(AC%ICU)))
          DO I=1,SIZE(AC%ICU)
             IICU(I)=NAMEIDX(AC%ICU(I),AC%parnames)
          ENDDO

          CALL AUTOI(AC,IICU)
          DEALLOCATE(IICU)
!-----------------------------------------------------------------------

          IF(MPIKWT()>1)THEN
             CALL MPITIM(TIME1)
          ELSE
             TIME1=AUTIM()
!$           TIME1=omp_get_wtime()
          ENDIF
          TOTTIM=TIME1-TIME0
          IF(AC%AP%IID>0)THEN
             CALL WRBAR(AC,"=",47)
             WRITE(AC%DUNIT,301)TOTTIM
          ENDIF
          WRITE(6,301)TOTTIM
          CALL CLEANUP(AC)
          IF(KEYS)EXIT
       ENDDO
       CALL AUTOSTOP()

 301  FORMAT(/,' Total Time ',E12.3)

      END SUBROUTINE AUTENTRY

!     ---------- ---------
      SUBROUTINE MPIWORKER(AC)
      
      USE AUTOMPI
      IMPLICIT NONE

      TYPE(AUTOCONTEXT) AC
      INTEGER, ALLOCATABLE :: ICU(:)

      DO WHILE(.TRUE.)
         CALL MPIBCASTAP(AC%AP)
         ALLOCATE(ICU(AC%AP%NICP))
         CALL AUTOI(AC,ICU)
         DEALLOCATE(ICU)
         ! autoi eventually calls autobv with the subroutines based on
         ! ap, which eventually calls solvbv;
         ! a return means another init message
      ENDDO
      END SUBROUTINE MPIWORKER

!     ---------- --------------
      SUBROUTINE FINDLB_OR_STOP(AC,UNITC)

! Find restart label and determine type of restart point.
! or stop otherwise

      IMPLICIT NONE
      TYPE(AUTOCONTEXT), INTENT(INOUT), TARGET :: AC
      TYPE(AUTOPARAMETERS), POINTER :: AP
      INTEGER, INTENT(IN) :: UNITC

      INTEGER NFPR,NPARR,IRS
      LOGICAL FOUND

      AP=>AC%AP

      IRS=AP%IRS

      FOUND=.FALSE.
      IF(IRS/=0) THEN
         CALL FINDLB(AC,UNITC,IRS,NFPR,NPARR,FOUND)
         AP%IRS=IRS
         AP%NFPR=NFPR
         IF(.NOT.FOUND) THEN
            WRITE(6,"(' Restart label ',A,' not found')")TRIM(AC%SIRS)
            CALL AUTOSTOP()
         ENDIF
         AP%NPAR=MAX(NPARR,AP%NPAR)
      ENDIF
      END SUBROUTINE FINDLB_OR_STOP

!     ---------- -----
      SUBROUTINE AUTOI(AC,ICU)

      USE TOOLBOXAE
      USE TOOLBOXBV
      USE EQUILIBRIUM
      USE MAPS
      USE OPTIMIZATION
      USE PARABOLIC
      USE PERIODIC
      USE HOMCONT
      USE TIMEINT

      IMPLICIT NONE
      TYPE(AUTOCONTEXT), INTENT(INOUT), TARGET :: AC
      TYPE(AUTOPARAMETERS), POINTER :: AP
      INTEGER ICU(AC%AP%NICP)

      INTEGER IPS,ISW,NNICP,NPAR
      INTEGER, ALLOCATABLE :: ICP(:)

      AP=>AC%AP

      IPS=AP%IPS
      ISW=AP%ISW

      ! transfer ICU array on MPI so the AUTO** subroutines can do their
      ! work normally on the workers.
      CALL MPIBCASTI(ICU,AP%NICP)
      NNICP=MAX(5*(AC%NBC+AC%NINT-AC%NDIM+1)+AC%NDIM+AC%NINT+3,5*SIZE(ICU)+AC%NDIM+3)
      ALLOCATE(ICP(NNICP))
      ICP(:SIZE(ICU))=ICU(:)
      ICP(SIZE(ICU)+1:)=0
      NPAR=AP%NPAR
      NPAR=MAX(MAXVAL(ABS(ICU)),NPAR)
      AP%NPAR=NPAR

      CALL INIT1(AC)

      SELECT CASE(IPS)
      CASE(0)
         ! general algebraic equations (no Hopf, eigenvalues, stability)
         CALL AUTOAEP(AC,ICP,ICU)
      CASE(1)
         ! equilibria
         CALL AUTOEQ(AC,ICP,ICU)
      CASE(-1)
         ! fixed points in maps
         CALL AUTODS(AC,ICP,ICU)
      CASE(2,4,7)
         ! periodic solutions and general BVPs
         IF(IPS==2.OR.(IPS==7.AND.ABS(ISW)<=1))THEN
            CALL AUTOPS(AC,ICP,ICU)
         ELSE
            CALL AUTOBVP(AC,ICP,ICU)
         ENDIF 
      CASE(-2)
         ! time integration
         CALL AUTOTI(AC,ICP,ICU)
      CASE(11,12,14,16,17)
         ! parabolic PDEs
         CALL AUTOPE(AC,ICP,ICU)
      CASE(5,15)
         ! optimization
         CALL AUTOOP(AC,ICP,ICU)
      CASE(9)
         ! Homoclinic bifurcation analysis.
         CALL AUTOHO(AC,ICP,ICU)
      END SELECT

      IF(AP%NTOT==0.AND.MPIIAM()==0)THEN
!        ** Error in INIT.
         WRITE(6,500)
         CALL AUTOSTOP()
      ENDIF

! Error Message.
 500  FORMAT(' Initialization Error')

      DEALLOCATE(ICP)

      END SUBROUTINE AUTOI
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!                    Initialization
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------

!     ---------- ----
      SUBROUTINE INIT(AC,UNITC,EOF,KEYS,LINE)

      USE HOMCONT, ONLY : INSTRHO

      IMPLICIT NONE

      TYPE(AUTOCONTEXT), INTENT(INOUT), TARGET :: AC
      TYPE(AUTOPARAMETERS), POINTER :: AP
      INTEGER, INTENT(IN) :: UNITC
      LOGICAL, INTENT(OUT) :: EOF
      LOGICAL, INTENT(INOUT) :: KEYS
      INTEGER, INTENT(INOUT) :: LINE

      INTEGER NPOS, IERR, KEYEND, POS, LISTLEN, I
      CHARACTER(LEN=2048) :: STR

      AP=>AC%AP

!     set default values
      AC%NDIM = 2
      AC%IPS  = 1
      AC%ILP  = 1
      AC%NTST = 20
      AC%NCOL = 4
      AC%IAD  = 3
      AC%IADS = 1
      AC%ISP  = 2
      AC%ISW  = 1
      AC%IPLT = 0
      AC%NBC  = 0
      AC%NINT = 0
      AC%NMX  = 0
      AC%NPR  = 0
      AC%MXBF = 10
      AC%IIS  = 3
      AC%IID  = 2
      AC%ITMX = 9
      AC%ITNW = 5
      AC%NWTN = 3
      AC%JAC  = 0
      AC%NPAR = NPARX
      AC%IBR  = 0
      AC%LAB  = 0

      AC%DS    = 0.01d0
      AC%DSMIN = 0.005d0
      AC%DSMAX = 0.1d0
      AC%RL0   = -HUGE(1d0)*0.99995d0 !avoid rounding up in sthd
      AC%RL1   = HUGE(1d0)*0.99995d0
      AC%A0    = -HUGE(1d0)*0.99995d0
      AC%A1    = HUGE(1d0)*0.99995d0
      AC%EPSL  = 1d-7
      AC%EPSU  = 1d-7
      AC%EPSS  = 1d-5

      AC%TY='' 
      AC%EFILE=''
      AC%SFILE=''
      AC%SVFILE=''
      AC%DATFILE=''

      ALLOCATE(AC%ICU(1),AC%IVUZR(0),AC%IVUZSTOP(0),AC%IVTHU(0), &
           AC%parnames(0),AC%unames(0),AC%SP(0))
      ALLOCATE(AC%STOPS(0),AC%UVALS(0),AC%PARVALS(0))
      AC%ICU(1)='1'

      NPOS=1
      DO
         CALL READC(AC,UNITC,EOF,LINE,NPOS,STR,KEYEND,POS,LISTLEN,IERR)
         ! IERR=-1: old-style constants file detected and read
         ! IERR= 0: no problems
         ! IERR= 1: unknown AUTO constant: check with HomCont
         ! IERR= 3: bad value
         IF(EOF.OR.IERR==-1)EXIT
         KEYS=.TRUE.
         IF(IERR==1)THEN
            CALL INSTRHO(AC,STR(1:KEYEND),STR(POS:),LISTLEN,IERR)
         ENDIF
         IF(IERR==1)THEN
            WRITE(6,'(A,A,A,I2)')"Unknown AUTO constant ", &
                 STR(1:KEYEND)," on line ",LINE
            CALL AUTOSTOP()
         ELSEIF(IERR==3)THEN
            WRITE(6,"(A,I2,A)") &
                 " Error in fort.2 or c. file: bad value on line ", LINE,"."
            CALL AUTOSTOP()
         ENDIF
      ENDDO

      IF(EOF.AND.IERR/=-1.AND..NOT.KEYS)THEN
         RETURN
      ENDIF

      AP%NDIM=AC%NDIM
      AP%IPS=AC%IPS
      AP%IRS=AC%IRS
      AP%ILP=AC%ILP
      AP%NTST=AC%NTST
      AP%NCOL=AC%NCOL
      AP%IAD=AC%IAD
      AP%IADS=AC%IADS
      AP%ISP=AC%ISP
      AP%ISW=AC%ISW
      AP%IPLT=AC%IPLT
      AP%NBC=AC%NBC
      AP%NINT=AC%NINT
      AP%NMX=AC%NMX
      AP%NUZR=0
      DO I=1,SIZE(AC%IVUZR)
         AP%NUZR=AP%NUZR+SIZE(AC%IVUZR(I)%VAR)
      ENDDO
      DO I=1,SIZE(AC%IVUZSTOP)
         AP%NUZR=AP%NUZR+SIZE(AC%IVUZSTOP(I)%VAR)
      ENDDO
      AP%NPR=AC%NPR
      AP%MXBF=AC%MXBF
      AP%IIS=AC%IIS
      AP%IID=AC%IID
      AP%ITMX=AC%ITMX
      AP%ITNW=AC%ITNW
      AP%NWTN=AC%NWTN      
      AP%JAC=AC%JAC
      AP%NPAR=AC%NPAR
      AP%IBR=AC%IBR
      AP%LAB=AC%LAB
      AP%NICP=SIZE(AC%ICU)
      AP%NTEST=2 ! LP/BP test functions active by default

      AP%NDM=AC%NDIM
      AP%NPARI=0
      AP%ITP=0
      AP%ITPST=0
      AP%NFPR=1
      AP%NTOT=0
      AP%NINS=0

      AP%DS=AC%DS
      AP%DSMIN=ABS(AC%DSMIN)
      AP%DSMAX=ABS(AC%DSMAX)
      AP%RDS=AC%DS
      AP%RL0=AC%RL0
      AP%RL1=AC%RL1
      AP%A0=AC%A0
      AP%A1=AC%A1

      AP%EPSL=AC%EPSL
      AP%EPSU=AC%EPSU
      AP%EPSS=AC%EPSS
      AP%DET=0.d0
      AP%FLDF=0.d0
      AP%HBFF=0.d0
      AP%BIFF=0.d0
      AP%SPBF=0.d0

      EOF=.FALSE.

      END SUBROUTINE INIT

!     ---------- -------
      SUBROUTINE CLEANUP(AC)

!     Deallocate some globally allocated arrays.
        
      IMPLICIT NONE
      TYPE(AUTOCONTEXT) AC
      INTEGER I

      DO I=1,SIZE(AC%IVUZR)
         DEALLOCATE(AC%IVUZR(I)%VAR)
      ENDDO
      DO I=1,SIZE(AC%IVUZSTOP)
         DEALLOCATE(AC%IVUZSTOP(I)%VAR)
      ENDDO
      DEALLOCATE(AC%IVTHU,AC%IVUZR,AC%IVUZSTOP,AC%IVTHL, &
           AC%ICU,AC%parnames,AC%unames,AC%SP,AC%STOPS, &
           AC%PARVALS,AC%UVALS)
      END SUBROUTINE CLEANUP

!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!               The leading subroutines of AUTO
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------

!     ---------- -----
      SUBROUTINE INIT1(AC)

      USE SUPPORT, ONLY: LBTYPE

      DOUBLE PRECISION, PARAMETER :: HMACH=1.0d-7

! General initialization. Redefinition of constants.
! The following constants are redefined, ie. they are different than in
! fort.2 or c.*:

!   DS: if DS is set to 0 it'll be set to 0.1
!   DS: if DSMIN is set to 0 it'll be set to 1.0d-4 * |DS|
!   DSMIN is divided by 1+HMACH
!   DS and DSMAX are multiplied by 1+HMACH

!   NDIM: set to the dimension of the extended system
!   ILP: set to 0 dependent on problem type
!   ISP: set to 0 dependent on problem type
!   ISW: set to 1 if equal to 0, to -|ISW| for starts of ext systems
!   NBC: set by problem type
!   NINT: set by problem type
!   NMX: set to 5 for starts of extended systems

      TYPE(AUTOCONTEXT), INTENT(INOUT), TARGET :: AC
      TYPE(AUTOPARAMETERS), POINTER :: AP

! Local
      DOUBLE PRECISION DS,DSMIN,FC

      AP=>AC%AP

       DS=AP%DS
       DSMIN=AP%DSMIN

       IF(AP%ISW.EQ.0)AP%ISW=1

! Check and perturb pseudo arclength stepsize and steplimits.
! (Perturbed to avoid exact computation of certain singular points).

       IF(DS.EQ.0.d0)DS=0.1
       IF(DSMIN.EQ.0.d0)DSMIN=1.0D-4*ABS(DS)
       FC=1.d0+HMACH
       AP%DS=FC*DS
       AP%DSMIN=DSMIN/FC
       AP%DSMAX=FC*AP%DSMAX
       AP%NPARI=0
       IF(.NOT.ALLOCATED(AC%IVTHL))THEN
          ! set default for *THL
          IF(AP%IPS==2.OR.AP%IPS==12)THEN
             ALLOCATE(AC%IVTHL(1))
             AC%IVTHL(1)%INDEX='11'
             AC%IVTHL(1)%VAR=0d0
          ELSE
             ALLOCATE(AC%IVTHL(0))
          ENDIF
       ENDIF

!translate TY constant to typecode in AP%ITP
       IF(LEN_TRIM(AC%TY)>=2)THEN
          DO I=-9,9
             IF(LBTYPE(I)==AC%TY(1:2))THEN
                AP%ITP=I
                EXIT
             ENDIF
          ENDDO
          IF(AC%TY(1:2)=='GH')THEN
             AP%ITP=-32
          ENDIF
          IF(.NOT.(AP%IPS<=1.OR.AP%IPS==5.OR.AP%IPS==11))THEN
             IF(AP%ITP==1)THEN
                AP%ITP=6
             ELSEIF(AP%ITP==2)THEN
                AP%ITP=5
             ENDIF
          ENDIF
       ENDIF

      RETURN
      END SUBROUTINE INIT1

      END MODULE AUTO
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
