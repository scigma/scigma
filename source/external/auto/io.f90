!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!                    Output (Algebraic Problems)
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------

MODULE IO

  USE AUTO_TYPES, ONLY: AUTOCONTEXT,AUTOPARAMETERS,SOLUTION

  IMPLICIT NONE
  PRIVATE
  PUBLIC :: READC, FINDLB, READLB, READBV, WRLINE, WRBAR, STHD, NEWLAB, &
       GETNDIM3, GETNTST3, GETNCOL3, GETNFPR3, GETIPS3
CONTAINS

! ------------- -------- -------
  CHARACTER(13) FUNCTION getname(is, ind)
    USE AUTO_TYPES, ONLY: INDEXSTR
    TYPE(INDEXSTR), INTENT(IN) :: is(:)
    INTEGER, INTENT(IN) :: ind
    INTEGER i

    getname = ''
    DO i = 1, SIZE(is)
       IF (is(i)%index == ind) THEN
          getname = is(i)%str
          RETURN
       ENDIF
    ENDDO
  END FUNCTION getname

! ---------- -----
  SUBROUTINE READC(AC,UNITC,EOF,LINE,NPOS,STR,KEYEND,POS,LISTLEN,IERR)

! Reads the file of continuation constants

    TYPE(AUTOCONTEXT), INTENT(INOUT) :: AC
    INTEGER, INTENT(IN) :: UNITC
    LOGICAL, INTENT(OUT) :: EOF
    INTEGER, INTENT(INOUT) :: LINE
    CHARACTER(*), INTENT(INOUT) :: STR
    INTEGER, INTENT(INOUT) :: NPOS
    INTEGER, INTENT(OUT) :: KEYEND, POS, LISTLEN, IERR

    INTEGER I,J,IC,N
    INTEGER NICP
    DOUBLE PRECISION RC
    CHARACTER(LEN=1) :: C,QUOTE,PREV
    LOGICAL QUOTEESC
    INTEGER LISTLEN2,ios

    TYPE INDEXSTRL
       CHARACTER(13) INDEX
       CHARACTER(2048) STRL
    END TYPE INDEXSTRL
    TYPE(INDEXSTRL),ALLOCATABLE :: IVUZRS(:)

    CHARACTER(LEN=*), PARAMETER :: ICONSTANTS(24) = (/                    &
         "NDIM", "IPS ", "ILP ", "NTST", "NCOL", "IAD ", "IADS", "ISP ",  &
         "ISW ", "IPLT", "NBC ", "NINT", "NMX ", "NPR ", "MXBF", "IID ",  &
         "ITMX", "ITNW", "NWTN", "JAC ", "NPAR", "IBR ", "LAB ", "IIS " /)
    CHARACTER(LEN=*), PARAMETER :: RCONSTANTS(10) = (/                    &
         "DS   ", "DSMIN", "DSMAX", "RL0  ", "RL1  ", "A0   ", "A1   ",   &
         "EPSL ", "EPSU ", "EPSS " /)

    EOF=.FALSE.
    POS=0
    KEYEND=0
    LISTLEN=0
    IERR=0

    IF(NPOS==1)THEN
       LINE=LINE+1
       N=READINTEGER(UNITC,STR(1:1),IERR)
       IF(IERR/=0)THEN
          IF(IERR==4)THEN
             IERR=0
             EOF=.TRUE.
          ELSEIF(IERR==-1)THEN
             CALL READOLDC(AC,UNITC,N,EOF,LINE,IERR)
          ENDIF
          RETURN
       ENDIF
       AC%NEWCFILE=.TRUE.
       READ(UNITC,'(A)',IOSTAT=ios) STR(2:)
       IF(ios/=0)THEN
          EOF=.TRUE.
          RETURN
       ENDIF
       QUOTE=' '
       QUOTEESC=.FALSE.
       DO I=1,LEN_TRIM(STR)
          C=STR(I:I)
          IF(QUOTE==' ')THEN
             ! replace a tab with spaces if not in a string
             IF(IACHAR(C)==9)THEN
                STR(I:I)=' '
             ELSEIF(C=="'".OR.C=='"')THEN
                QUOTE=STR(I:I)
             ENDIF
          ELSEIF(C==QUOTE)THEN
             ! ignore "" and ''
             IF(STR(I+1:I+1)==C.OR.QUOTEESC)THEN
                QUOTEESC=.NOT.QUOTEESC
             ELSE
                QUOTE=' '
             ENDIF
          ENDIF
       ENDDO
    ELSE
       STR=STR(NPOS:)
    ENDIF
    STR=ADJUSTL(STR)
    IF(LEN_TRIM(STR)==0)RETURN
    ! comment on line
    IF(STR(1:1)=='#'.OR.STR(1:1)=='!')THEN
       NPOS=1
       RETURN
    ENDIF
    ! look for = after keyword
    KEYEND=SCAN(STR,'= ')-1
    IF(KEYEND==-1)GOTO 3
    POS=SCAN(STR,'=')+1
    STR(POS:)=ADJUSTL(STR(POS:))
    CALL SCANVALUE(UNITC,STR(POS:),NPOS,LISTLEN)
    IF(NPOS/=1)THEN
       NPOS=NPOS+POS-1
    ENDIF
    DO I=1,SIZE(ICONSTANTS)
       IF(STR(1:KEYEND)==TRIM(ICONSTANTS(I)))THEN
          READ(STR(POS:),*,ERR=3)IC
          SELECT CASE(I)
          CASE(1)
             AC%NDIM=IC
          CASE(2)
             AC%IPS=IC
          CASE(3)
             AC%ILP=IC
          CASE(4)
             AC%NTST=IC
          CASE(5)
             AC%NCOL=IC
          CASE(6)
             AC%IAD=IC
          CASE(7)
             AC%IADS=IC
          CASE(8)
             AC%ISP=IC
          CASE(9)
             AC%ISW=IC
          CASE(10)
             AC%IPLT=IC
          CASE(11)
             AC%NBC=IC
          CASE(12)
             AC%NINT=IC
          CASE(13)
             AC%NMX=IC
          CASE(14)
             AC%NPR=IC
          CASE(15)
             AC%MXBF=IC
          CASE(16)
             AC%IID=IC
          CASE(17)
             AC%ITMX=IC
          CASE(18)
             AC%ITNW=IC
          CASE(19)
             AC%NWTN=IC
          CASE(20)
             AC%JAC=IC
          CASE(21)
             AC%NPAR=IC
          CASE(22)
             AC%IBR=IC
          CASE(23)
             AC%LAB=IC
          CASE(24)
             AC%IIS=IC
          END SELECT
          RETURN
       ENDIF
    ENDDO
    DO I=1,SIZE(RCONSTANTS)
       IF(STR(1:KEYEND)==TRIM(RCONSTANTS(I)))THEN
          READ(STR(POS:),*,ERR=3)RC
          SELECT CASE(I)
          CASE(1)
             AC%DS=RC
          CASE(2)
             AC%DSMIN=RC
          CASE(3)
             AC%DSMAX=RC
          CASE(4)
             AC%RL0=RC
          CASE(5)
             AC%RL1=RC
          CASE(6)
             AC%A0=RC
          CASE(7)
             AC%A1=RC
          CASE(8)
             AC%EPSL=RC
          CASE(9)
             AC%EPSU=RC
          CASE(10)
             AC%EPSS=RC
          END SELECT
          RETURN
       ENDIF
    ENDDO
    SELECT CASE(STR(1:KEYEND))
    CASE('IRS')
       READ(STR(POS:),*,ERR=3)AC%SIRS
       READ(AC%SIRS,*,IOSTAT=ios)AC%IRS
       IF(ios/=0)AC%IRS=1
    CASE('ICP')
       NICP=LISTLEN
       DEALLOCATE(AC%ICU)
       ALLOCATE(AC%ICU(NICP))
       READ(STR(POS:),*,ERR=3)AC%ICU            
    CASE('UZR','UZSTOP')
       ALLOCATE(IVUZRS(LISTLEN))
       READ(STR(POS:),*,ERR=3)IVUZRS
       IF(STR(1:KEYEND)=='UZSTOP')THEN
          DO I=1,SIZE(AC%IVUZSTOP)
             DEALLOCATE(AC%IVUZSTOP(I)%VAR)
          ENDDO
          DEALLOCATE(AC%IVUZSTOP)
          ALLOCATE(AC%IVUZSTOP(LISTLEN))
       ELSE
          DO I=1,SIZE(AC%IVUZR)
             DEALLOCATE(AC%IVUZR(I)%VAR)
          ENDDO
          DEALLOCATE(AC%IVUZR)
          ALLOCATE(AC%IVUZR(LISTLEN))
       ENDIF
       DO I=1,LISTLEN
          PREV=' '
          LISTLEN2=0
          DO J=1,LEN_TRIM(IVUZRS(I)%STRL)
             C=IVUZRS(I)%STRL(J:J)
             IF(C/=' '.AND.C/=','.AND.(PREV==' '.OR.PREV==','))THEN
                LISTLEN2=LISTLEN2+1
             ENDIF
             PREV=C
          ENDDO
          IF(STR(1:KEYEND)=='UZSTOP')THEN
             ALLOCATE(AC%IVUZSTOP(I)%VAR(LISTLEN2))
             AC%IVUZSTOP(I)%INDEX=IVUZRS(I)%INDEX
             READ(IVUZRS(I)%STRL,*,ERR=3)AC%IVUZSTOP(I)%VAR
          ELSE
             ALLOCATE(AC%IVUZR(I)%VAR(LISTLEN2))
             AC%IVUZR(I)%INDEX=IVUZRS(I)%INDEX
             READ(IVUZRS(I)%STRL,*,ERR=3)AC%IVUZR(I)%VAR
          ENDIF
       ENDDO
       DEALLOCATE(IVUZRS)
    CASE('THL')
       IF(ALLOCATED(AC%IVTHL))DEALLOCATE(AC%IVTHL)
       ALLOCATE(AC%IVTHL(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%IVTHL
    CASE('THU')
       DEALLOCATE(AC%IVTHU)
       ALLOCATE(AC%IVTHU(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%IVTHU
    CASE('SP')
       IF(ALLOCATED(AC%SP))DEALLOCATE(AC%SP)
       ALLOCATE(AC%SP(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%SP
    CASE('STOP')
       IF(ALLOCATED(AC%STOPS))DEALLOCATE(AC%STOPS)
       ALLOCATE(AC%STOPS(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%STOPS
    CASE('PAR')
       IF(ALLOCATED(AC%PARVALS))DEALLOCATE(AC%PARVALS)
       ALLOCATE(AC%PARVALS(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%PARVALS
    CASE('U')
       IF(ALLOCATED(AC%UVALS))DEALLOCATE(AC%UVALS)
       ALLOCATE(AC%UVALS(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%UVALS
    CASE('parnames')
       IF(ALLOCATED(AC%parnames))DEALLOCATE(AC%parnames)
       ALLOCATE(AC%parnames(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%parnames
    CASE('unames')
       IF(ALLOCATED(AC%unames))DEALLOCATE(AC%unames)
       ALLOCATE(AC%unames(LISTLEN))
       READ(STR(POS:),*,ERR=3)AC%unames
    CASE('s')
       READ(STR(POS:),*)AC%SFILE
       IF(TRIM(AC%SFILE)=='/')THEN
          ! special case from Python interface: s='/' is followed on
          ! the next lines by the solution
          EOF=.TRUE.
          RETURN
       ENDIF
    CASE('dat')
       READ(STR(POS:),*)AC%DATFILE
    CASE('sv')
       READ(STR(POS:),*)AC%SVFILE
    CASE('e')
       READ(STR(POS:),*)AC%EFILE
    CASE('TY')
       READ(STR(POS:),*,ERR=3)AC%TY
    CASE DEFAULT
       IERR=1
    END SELECT
    RETURN
3   IERR=3
  END SUBROUTINE READC

! ---------- ---------
  SUBROUTINE SCANVALUE(UNITC,STR,NPOS,LISTLEN)
    IMPLICIT NONE

!   Scans STR(:) for a value
!   NPOS points to the next keyword on the same line,
!     or is set to 1 if there is none
!   LISTLEN gives the number of items in lists delimited by [] or {}
!   [] characters are removed

    INTEGER, INTENT(IN) :: UNITC
    CHARACTER(*), INTENT(INOUT) :: STR
    INTEGER, INTENT(OUT) :: NPOS,LISTLEN

    INTEGER I,LEVEL,LENSTR,ios
    CHARACTER(1) C,PREV,QUOTE
    LOGICAL QUOTEESC,ISDICT
    LISTLEN=1
    LEVEL=0
    QUOTE=' '
    QUOTEESC=.FALSE.
    PREV=' '

    NPOS=1
    ISDICT=.FALSE.
    LENSTR=LEN_TRIM(STR)
    I=1
    DO
       IF(I>LENSTR)THEN
          IF(LEVEL==0)EXIT
          LENSTR=LEN_TRIM(STR)
          READ(UNITC,'(A)',IOSTAT=ios) STR(LENSTR+1:)
          IF(ios/=0)EXIT
          LENSTR=LEN_TRIM(STR)
       ENDIF
       NPOS=I
       C=STR(I:I)
       IF(QUOTE==' ')THEN
          SELECT CASE(C)
          CASE(',',' ')
             IF(LEVEL==0)EXIT
             IF(PREV==':'.OR.PREV=='['.OR.PREV=='{')C=PREV
             !eat ',' and ' ' after ':', '[', or '{'
          CASE(':')
             STR(I:I)=','
          CASE(']','}')
             IF(C=='}') ISDICT=.FALSE.
             STR(I:I)=' '
             IF(LEVEL==1.AND.(PREV=='['.OR.PREV=='{'))LISTLEN=0
             LEVEL=LEVEL-1
             IF(C==']'.AND.ISDICT) STR(I:I)="'"
          CASE DEFAULT
             IF((PREV==','.OR.PREV==' ').AND.LEVEL==1)THEN
                LISTLEN=LISTLEN+1
             ENDIF
             SELECT CASE(C)
             CASE('[','{')
                STR(I:I)=' '
                LEVEL=LEVEL+1
                IF(C=='{')THEN
                   ISDICT=.TRUE.
                ELSEIF(ISDICT)THEN
                   STR(I:I)="'"
                ENDIF
             CASE('"',"'")
                QUOTE=C
             END SELECT
          END SELECT
       ELSEIF(C==QUOTE)THEN
          ! ignore "" and ''
          IF(STR(I+1:I+1)==C.OR.QUOTEESC)THEN
             QUOTEESC=.NOT.QUOTEESC
          ELSE
             QUOTE=' '
          ENDIF
       ENDIF
       PREV=C
       I=I+1
    ENDDO
    I=VERIFY(STR(NPOS:)," ,")
    IF(I==0)THEN
       NPOS=1
    ELSE
       NPOS=NPOS+I-1
       IF(NPOS>=LEN_TRIM(STR))NPOS=1
    ENDIF
  END SUBROUTINE SCANVALUE

! ------- -------- -----------
  INTEGER FUNCTION READINTEGER(UNITC,C,IERR)

    ! read arbitrary positive integer from unit 2 without advancing
    ! input, to work around advance='no' restriction for list-directed
    ! input.
    ! on output: C contains the last-read character
    ! ierr:-1: no problems and number read
    !       0: no number read
    !       3: error reading
    !       4: eof
    INTEGER, INTENT(IN) :: UNITC
    CHARACTER(1), INTENT(OUT) :: C
    INTEGER, INTENT(OUT) :: IERR

    INTEGER N

    READINTEGER = 0

    N = 0
    IERR = 0
    DO
       READ(UNITC,'(A1)',ERR=3,END=4,EOR=5,ADVANCE='NO')C
       IF(IERR==0.AND.(IACHAR(C)==9.OR.C==' '))CYCLE
       IF(LLT(C, '0').OR.LGT(C, '9'))EXIT
       IERR = -1
       N = N*10 + IACHAR(C) - IACHAR('0')
    ENDDO
    READINTEGER = N
    RETURN

3   IERR = 3
    RETURN

4   IERR = 4
5   RETURN

  END FUNCTION READINTEGER

! ---------- --------
  SUBROUTINE READOLDC(AC,UNITC,N,EOF,LINE,IERR)

! Reads the continuation constants in the old format
    
    TYPE(AUTOCONTEXT), INTENT(INOUT) :: AC
    INTEGER I
    INTEGER NUZR,NICP
    INTEGER LISTLEN,ios
    CHARACTER(1) C

    INTEGER, INTENT(IN) :: UNITC, N
    LOGICAL, INTENT(INOUT) :: EOF
    INTEGER, INTENT(INOUT) :: LINE
    INTEGER, INTENT(OUT) :: IERR

    AC%NDIM=N
    READ(UNITC,*,ERR=3,END=4) AC%IPS,AC%SIRS,AC%ILP
    READ(AC%SIRS,*,IOSTAT=ios)AC%IRS
    IF(ios/=0)AC%IRS=1
    LINE=LINE+1
    LISTLEN=READINTEGER(UNITC,C,IERR)
    IF(IERR==0.OR.IERR==3)GOTO 3
    IF(IERR==4)GOTO 4
    DEALLOCATE(AC%ICU)
    NICP=LISTLEN
    IF(LISTLEN==0)NICP=1
    ALLOCATE(AC%ICU(NICP))
    AC%ICU(1)='1'
    READ(UNITC,*,ERR=3,END=4) (AC%ICU(I),I=1,LISTLEN)
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) AC%NTST,AC%NCOL,AC%IAD,AC%ISP,AC%ISW,AC%IPLT,AC%NBC,AC%NINT
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) AC%NMX,AC%RL0,AC%RL1,AC%A0,AC%A1
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) AC%NPR,AC%MXBF,AC%IID,AC%ITMX,AC%ITNW,AC%NWTN,AC%JAC
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) AC%EPSL,AC%EPSU,AC%EPSS
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) AC%DS,AC%DSMIN,AC%DSMAX,AC%IADS
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) LISTLEN
    !allocate: no THL vs. non-allocated:default THL (in SUB. INIT1)
    IF(ALLOCATED(AC%IVTHL))DEALLOCATE(AC%IVTHL)
    ALLOCATE(AC%IVTHL(LISTLEN))
    IF(LISTLEN>0)THEN
       DO I=1,LISTLEN
          LINE=LINE+1
          READ(UNITC,*,ERR=3,END=4)AC%IVTHL(I)
       ENDDO
    ENDIF
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4) LISTLEN
    IF(LISTLEN>0)THEN
       DEALLOCATE(AC%IVTHU)
       ALLOCATE(AC%IVTHU(LISTLEN))
       DO I=1,LISTLEN
          LINE=LINE+1
          READ(UNITC,*,ERR=3,END=4)AC%IVTHU(I)
       ENDDO
    ENDIF
    LINE=LINE+1
    READ(UNITC,*,ERR=3,END=4)NUZR
    IF(NUZR>0)THEN
       DO I=1,SIZE(AC%IVUZR)
          DEALLOCATE(AC%IVUZR(I)%VAR)
       ENDDO
       DEALLOCATE(AC%IVUZR)
       ALLOCATE(AC%IVUZR(NUZR))
       DO I=1,NUZR
          LINE=LINE+1
          ALLOCATE(AC%IVUZR(I)%VAR(1))
          READ(UNITC,*,ERR=3,END=4)AC%IVUZR(I)%INDEX,AC%IVUZR(I)%VAR(1)
       ENDDO
    ENDIF
    IERR=-1
    RETURN

3   IERR=3
    RETURN

4   WRITE(6,"(A,I2,A)") &
         " Error in fort.2 or c. file: ends prematurely on line ", LINE,"."
    EOF=.TRUE.

  END SUBROUTINE READOLDC
    
! ---------- ----
  SUBROUTINE STHD(AC,ICP)

    USE COMPAT
    USE SUPPORT, ONLY: LBTYPE

! Write the values of the user defined parameters on unit 7.
! This identifying information is preceded by a '   0' on each line.
! The first line in the file contains the (generally) user-supplied
! limits of the bifurcation diagram, viz. RL0,RL1,A0 and A1.
! These are often convenient for an initial plot of the diagram.

    TYPE(AUTOCONTEXT), INTENT(IN), TARGET :: AC
    TYPE(AUTOPARAMETERS), POINTER :: AP
    INTEGER, INTENT(IN) :: ICP(*)
    CHARACTER (LEN=*), PARAMETER :: D3 = "('   0',3(A8,ES11.4))"
    CHARACTER (LEN=*), PARAMETER :: I4 = "('   0',4(A8,I4))"
    CHARACTER (LEN=*), PARAMETER :: I5 = "('   0',2(A7,I5),3(A8,I4))"
    CHARACTER (LEN=*), PARAMETER :: I6 = "('   0',5(A8,I4))"
    INTEGER NDIMA,IPS,IRSA,ILPA,NTST,NCOL,IAD,IADS,ISPA,ISWA,IPLT,NBCA,NINTA
    INTEGER NMXA,NUZR,NPR,MXBF,IIS,IID,ITMX,ITNW,NWTN,JAC,NFPR,I,NPARA
    INTEGER ITPST,LSV,LDAT,LE,LS,INDX,io
    DOUBLE PRECISION DSA,DSMINA,DSMAXA,RL0,RL1,A0,A1,EPSL,EPSU,EPSS
    CHARACTER(LEN=11) :: SDS, SDSA, SDSMAX, SDSMAXA, SDSMIN, SDSMINA
    CHARACTER(LEN=12) :: INDSTR
    CHARACTER(LEN=13) :: name

    AP=>AC%AP

    NDIMA=AP%NDIM
    IPS=AP%IPS
    IRSA=AP%IRS
    ILPA=AP%ILP
    NTST=AP%NTST
    NCOL=AP%NCOL
    IAD=AP%IAD
    IADS=AP%IADS
    ISPA=AP%ISP
    ISWA=AP%ISW
    IPLT=AP%IPLT
    NBCA=AP%NBC
    NINTA=AP%NINT
    NMXA=AP%NMX
    NUZR=AP%NUZR
    NPR=AP%NPR
    MXBF=AP%MXBF
    IIS=AP%IIS
    IID=AP%IID
    ITMX=AP%ITMX
    ITNW=AP%ITNW
    NWTN=AP%NWTN
    JAC=AP%JAC
    ITPST=AP%ITPST
    NFPR=AP%NFPR
    NPARA=AP%NPAR

    DSA=AP%DS
    DSMINA=AP%DSMIN
    DSMAXA=AP%DSMAX
    RL0=AP%RL0
    RL1=AP%RL1
    A0=AP%A0
    A1=AP%A1
    EPSL=AP%EPSL
    EPSU=AP%EPSU
    EPSS=AP%EPSS

    WRITE(AC%BUNIT,"(I4,' ',4ES12.4)")0,RL0,RL1,A0,A1
    WRITE(AC%BUNIT,D3)'EPSL=',EPSL,'EPSU =',EPSU, 'EPSS =',EPSS
    WRITE(AC%BUNIT,D3)'DS  =',DSA,  'DSMIN=',DSMINA,'DSMAX=',DSMAXA
    WRITE(AC%BUNIT,I4)'NDIM=',NDIMA,'IPS =',IPS, 'IRS =',IRSA, 'ILP =',ILPA
    WRITE(AC%BUNIT,I4)'NTST=',NTST,'NCOL=',NCOL,'IAD =',IAD, 'ISP =',ISPA
    WRITE(AC%BUNIT,I4)'ISW =',ISWA, 'IPLT=',IPLT,'NBC =',NBCA, 'NINT=',NINTA
    WRITE(AC%BUNIT,I5)' NMX=',NMXA, 'NPR=', NPR, 'MXBF=',MXBF,'IID =',IID, 'IADS=',IADS
    WRITE(AC%BUNIT,I6)'ITMX=',ITMX,'ITNW=',ITNW,'NWTN=',NWTN,'JAC =',JAC,'  NUZR=',NUZR

    IF(AC%IBR>0.OR.AC%LAB>0.OR.LEN_TRIM(AC%TY)>0.OR.IIS<3)THEN
       WRITE(AC%BUNIT,"('   0')",ADVANCE="NO")
       IF(IIS<3)THEN
          WRITE(AC%BUNIT,"(A8,I4)",ADVANCE="NO")"IIS =",IIS
       ENDIF
       IF(AC%IBR>0)THEN
          WRITE(AC%BUNIT,"(A8,I4)",ADVANCE="NO")"IBR =",AC%IBR
       ENDIF
       IF(AC%LAB>0)THEN
          WRITE(AC%BUNIT,"(A8,I4)",ADVANCE="NO")"LAB =",AC%LAB
       ENDIF
       IF(LEN_TRIM(AC%TY)>0)THEN
          WRITE(AC%BUNIT,"(A9,A,A)",ADVANCE="NO")"TY = '",TRIM(AC%TY),"'"
       ENDIF
       WRITE(AC%BUNIT,*)
    ENDIF
    WRITE(AC%BUNIT,"(A,I4,A)",ADVANCE="NO")"   0   NPAR=",NPARA
    CALL WRITELIST("   THL = ",AC%IVTHL)
    CALL WRITELIST("    THU = ",AC%IVTHU)
    WRITE(AC%BUNIT,*)
    IF(SIZE(AC%IVUZR)>0)THEN
       CALL WRITEUZRLIST("   0   UZR = ",AC%IVUZR)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%IVUZSTOP)>0)THEN
       CALL WRITEUZRLIST("   0   UZSTOP = ",AC%IVUZSTOP)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(IPS==9)THEN
       !homcont constants
       WRITE(AC%BUNIT,"('   0   ',2(A,I4),2(A8,I2),(A8,I4))") &
            'NUNSTAB=',AC%HOMCONT%NUNSTAB,' NSTAB=',AC%HOMCONT%NSTAB,&
            'IEQUIB=',AC%HCONST%IEQUIB,'ITWIST=',AC%HCONST%ITWIST,&
            'ISTART=',AC%HOMCONT%ISTART
       IF(SIZE(AC%HCONST%IREV)>0.OR.SIZE(AC%HCONST%IFIXED)>0.OR.&
            SIZE(AC%HCONST%IPSI)>0)THEN
          WRITE(AC%BUNIT,"('   0  ')",ADVANCE='NO')
          CALL WRITEINTLIST(" IREV=",AC%HCONST%IREV)
          CALL WRITEINTLIST(" IFIXED=",AC%HCONST%IFIXED)
          CALL WRITEINTLIST(" IPSI=",AC%HCONST%IPSI)
          WRITE(AC%BUNIT,*)
       ENDIF
    ENDIF
    LE=LEN_TRIM(AC%EFILE)
    LSV=LEN_TRIM(AC%SVFILE)
    LS=LEN_TRIM(AC%SFILE)
    LDAT=LEN_TRIM(AC%DATFILE)
    IF(LE>0.OR.LSV>0.OR.LS>0.OR.LDAT>0)THEN
       WRITE(AC%BUNIT,"('   0  ')",ADVANCE="NO")
       IF(LE>0)THEN
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" e = '",TRIM(AC%EFILE),"'"
       ENDIF
       IF(LS>0)THEN
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" s = '",TRIM(AC%SFILE),"'"
       ENDIF
       IF(LDAT>0)THEN
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" dat = '",TRIM(AC%DATFILE),"'"
       ENDIF
       IF(LSV>0)THEN
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" sv = '",TRIM(AC%SVFILE),"'"
       ENDIF
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%parnames)>0)THEN
       CALL WRITESTRLIST("   0   parnames = ",AC%parnames)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%unames)>0)THEN
       CALL WRITESTRLIST("   0   unames   = ",AC%unames)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%PARVALS)>0)THEN
       CALL WRITELIST("   0   PAR     = ",AC%PARVALS)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%UVALS)>0)THEN
       CALL WRITELIST("   0   U       = ",AC%UVALS)
       WRITE(AC%BUNIT,*)
    ENDIF
    IF(SIZE(AC%SP)>0)THEN
       WRITE(AC%BUNIT,"(A,A,A)", ADVANCE="NO")"   0   SP=['",TRIM(AC%SP(1)),"'"
       DO I=2,SIZE(AC%SP)
          WRITE(AC%BUNIT,"(A,A,A)", ADVANCE="NO")", '",TRIM(AC%SP(I)),"'"
       ENDDO
       WRITE(AC%BUNIT,"(A)")']'
    ENDIF
    IF(SIZE(AC%STOPS)>0)THEN
       WRITE(AC%BUNIT,"(A,A,A)", ADVANCE="NO")"   0   STOP=['",TRIM(AC%STOPS(1)),"'"
       DO I=2,SIZE(AC%STOPS)
          WRITE(AC%BUNIT,"(A,A,A)", ADVANCE="NO")", '",TRIM(AC%STOPS(I)),"'"
       ENDDO
       WRITE(AC%BUNIT,"(A)")']'
    ENDIF

    WRITE(SDS,    "(ES11.4)")AC%DS
    WRITE(SDSA,   "(ES11.4)")DSA
    WRITE(SDSMIN, "(ES11.4)")AC%DSMIN
    WRITE(SDSMINA,"(ES11.4)")DSMINA
    WRITE(SDSMAX, "(ES11.4)")AC%DSMAX
    WRITE(SDSMAXA,"(ES11.4)")DSMAXA

    IF(SDS/=SDSA.OR.SDSMIN/=SDSMINA.OR.SDSMAX/=SDSMAXA.OR. &
       AC%NDIM/=NDIMA.OR.AC%IRS/=IRSA.OR.AC%ILP/=ILPA.OR.AC%NPAR/=NPARA.OR. &
       AC%NMX/=NMXA.OR.AC%ISP/=ISPA.OR.AC%ISW/=ISWA.OR.AC%NBC/=NBCA.OR.AC%NINT/=NINTA)THEN
       WRITE(AC%BUNIT,"('   0   User-specified constants, where different from above:')")
       IF(SDS/=SDSA.OR.SDSMIN/=SDSMINA.OR.SDSMAX/=SDSMAXA)THEN
          WRITE(AC%BUNIT,"('   0')", ADVANCE="NO")
          IF(SDS/=SDSA)THEN
             WRITE(AC%BUNIT, "(A8,A)", ADVANCE="NO")'DS  =',SDS
          ENDIF
          IF(SDSMIN/=SDSMINA)THEN
             WRITE(AC%BUNIT, "(A8,A)", ADVANCE="NO")'DSMIN=',SDSMIN
          ENDIF
          IF(SDSMAX/=SDSMAXA)THEN
             WRITE(AC%BUNIT, "(A8,A)", ADVANCE="NO")'DSMAX=',SDSMAX
          ENDIF
          WRITE(AC%BUNIT,*)
       ENDIF
       IF(AC%NDIM/=NDIMA.OR.AC%IRS/=IRSA.OR.AC%ILP/=ILPA.OR.AC%NPAR/=NPARA)THEN
          WRITE(AC%BUNIT,"('   0')", ADVANCE="NO")
          IF(AC%NDIM/=NDIMA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'NDIM=',AC%NDIM
          ENDIF
          IF(AC%IRS/=IRSA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'IRS =',AC%IRS
          ENDIF
          IF(AC%ILP/=ILPA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'ILP =',AC%ILP
          ENDIF
          IF(AC%NPAR/=NPARA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'NPAR=',AC%NPAR
          ENDIF
          WRITE(AC%BUNIT,*)
       ENDIF
       IF(AC%NMX/=NMXA.OR.AC%ISP/=ISPA.OR.AC%ISW/=ISWA.OR.AC%NBC/=NBCA.OR.AC%NINT/=NINTA)THEN
          WRITE(AC%BUNIT,"('   0')", ADVANCE="NO")
          IF(AC%NMX/=NMXA)THEN
             WRITE(AC%BUNIT, "(A7,I5)", ADVANCE="NO")'NMX=',AC%NMX
          ENDIF
          IF(AC%ISP/=ISPA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'ISP =',AC%ISP
          ENDIF
          IF(AC%ISW/=ISWA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'ISW =',AC%ISW
          ENDIF
          IF(AC%NBC/=NBCA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'NBC =',AC%NBC
          ENDIF
          IF(AC%NINT/=NINTA)THEN
             WRITE(AC%BUNIT, "(A8,I4)", ADVANCE="NO")'NINT=',AC%NINT
          ENDIF
          WRITE(AC%BUNIT,*)
       ENDIF
    ENDIF
    IF(IPS==9.AND.(AC%HOMCONT%NUNSTAB/=AC%HCONST%NUNSTAB.OR.AC%HOMCONT%NSTAB/=AC%HCONST%NSTAB.OR. &
         AC%HOMCONT%ISTART/=AC%HCONST%ISTART))THEN
       !homcont constants
       WRITE(AC%BUNIT,"('   0   ')", ADVANCE="NO")
       IF(AC%HOMCONT%NUNSTAB/=AC%HCONST%NUNSTAB)THEN
          WRITE(AC%BUNIT,"(A,I4)", ADVANCE="NO")'NUNSTAB=',AC%HCONST%NUNSTAB
       ENDIF
       IF(AC%HOMCONT%NSTAB/=AC%HCONST%NSTAB)THEN
          WRITE(AC%BUNIT,"(A,I4)", ADVANCE="NO")' NSTAB=',AC%HCONST%NSTAB
       ENDIF
       IF(AC%HOMCONT%ISTART/=AC%HCONST%ISTART)THEN
          WRITE(AC%BUNIT,"(A8,I4)", ADVANCE="NO")'ISTART=',AC%HCONST%ISTART
       ENDIF
       WRITE(AC%BUNIT,*)
    ENDIF

    WRITE(AC%BUNIT,"('   0   User-specified parameter')",ADVANCE="NO")
    IF(SIZE(AC%ICU).EQ.1)THEN
       WRITE(AC%BUNIT,"(':       ')",ADVANCE="NO")
    ELSE
       WRITE(AC%BUNIT,"('s:      ')",ADVANCE="NO")
    ENDIF
    DO I=1,SIZE(AC%ICU)
       READ(AC%ICU(I),*,IOSTAT=io)INDX
       IF(io==0)THEN
          WRITE(AC%BUNIT,"(3X,A)",ADVANCE="NO")TRIM(AC%ICU(I))
       ELSE
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" '",TRIM(AC%ICU(I)),"'"
       ENDIF
    ENDDO
    WRITE(AC%BUNIT,"(/'   0   Active continuation parameter')",ADVANCE="NO")
    IF(NFPR.EQ.1)THEN
       WRITE(AC%BUNIT,"(':  ')",ADVANCE="NO")
    ELSE
       WRITE(AC%BUNIT,"('s: ')",ADVANCE="NO")
    ENDIF
    DO I=1,NFPR
       name = getname(AC%parnames, ICP(I))
       IF (LEN_TRIM(name)>0) THEN
          WRITE(AC%BUNIT,"(A,A,A)",ADVANCE="NO")" '",TRIM(name),"'"
       ELSE
          WRITE(INDSTR,"(I12)")ICP(I)
          WRITE(AC%BUNIT,"(3X,A)",ADVANCE="NO")TRIM(ADJUSTL(INDSTR))
       ENDIF
    ENDDO
    WRITE(AC%BUNIT,*)

    IF(ITPST /= 0)THEN
       WRITE(AC%BUNIT,"('   0   Branch type: ',I2,A,A,A)")&
            ITPST,"    TY = '",LBTYPE(ITPST),"'"
    ENDIF

    CALL AUTOFLUSH(AC%BUNIT)

  CONTAINS

    SUBROUTINE WRITELIST(NAME,IVLIST)
      USE AUTO_TYPES, ONLY: INDEXVAR
      CHARACTER(LEN=*), INTENT(IN) :: NAME
      TYPE(INDEXVAR), INTENT(IN) :: IVLIST(:)
      
      LOGICAL FIRST
      CHARACTER(LEN=15) :: INDSTR
      INTEGER INDX,io
      CHARACTER(LEN=19) :: VARSTR

      WRITE(AC%BUNIT,"(A,A)", ADVANCE="NO")NAME,'{'
      FIRST=.TRUE.
      DO I=1,SIZE(IVLIST)
         IF(.NOT.FIRST)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")", "
         READ(IVLIST(I)%INDEX,*,IOSTAT=io)INDX
         IF(io==0)THEN
            INDSTR=IVLIST(I)%INDEX
         ELSE
            INDSTR="'"//TRIM(IVLIST(I)%INDEX)//"'"
         ENDIF
         IF(INT(IVLIST(I)%VAR)==IVLIST(I)%VAR)THEN
            WRITE(VARSTR,'(I19)')INT(IVLIST(I)%VAR)
         ELSE
            WRITE(VARSTR,'(ES19.10)')IVLIST(I)%VAR
         ENDIF
         WRITE(AC%BUNIT,"(A,A,A)", ADVANCE="NO")TRIM(INDSTR),&
              ": ",TRIM(ADJUSTL(VARSTR))
         FIRST=.FALSE.
      ENDDO
      WRITE(AC%BUNIT,"(A)", ADVANCE="NO")'}'
    END SUBROUTINE WRITELIST

    SUBROUTINE WRITEUZRLIST(NAME,IVLIST)
      USE AUTO_TYPES, ONLY: INDEXMVAR
      CHARACTER(LEN=*), INTENT(IN) :: NAME
      TYPE(INDEXMVAR), INTENT(IN) :: IVLIST(:)
      
      LOGICAL FIRST
      CHARACTER(LEN=15) :: INDSTR
      INTEGER INDX,I,J,io
      CHARACTER(LEN=19) :: VARSTR
      DOUBLE PRECISION V

      WRITE(AC%BUNIT,"(A,A)", ADVANCE="NO")NAME,'{'
      FIRST=.TRUE.
      DO I=1,SIZE(IVLIST)
         IF(.NOT.FIRST)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")", "
         READ(IVLIST(I)%INDEX,*,IOSTAT=io)INDX
         IF(io==0)THEN
            INDSTR=IVLIST(I)%INDEX
         ELSE
            INDSTR="'"//TRIM(IVLIST(I)%INDEX)//"'"
         ENDIF
         WRITE(AC%BUNIT,"(A,A)", ADVANCE="NO")TRIM(INDSTR),": "
         IF(SIZE(IVLIST(I)%VAR)>1)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")'['
         DO J=1,SIZE(IVLIST(I)%VAR)
            IF(J>1)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")", "
            V=IVLIST(I)%VAR(J)
            IF(INT(V)==V)THEN
               WRITE(VARSTR,'(I19)')INT(V)
            ELSE
               WRITE(VARSTR,'(ES19.10)')V
            ENDIF
            WRITE(AC%BUNIT,"(A)", ADVANCE="NO")TRIM(ADJUSTL(VARSTR))
         ENDDO
         IF(SIZE(IVLIST(I)%VAR)>1)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")']'
         FIRST=.FALSE.
      ENDDO
      WRITE(AC%BUNIT,"(A)", ADVANCE="NO")'}'
    END SUBROUTINE WRITEUZRLIST

    SUBROUTINE WRITESTRLIST(NAME,ISLIST)
      USE AUTO_TYPES, ONLY: INDEXSTR
      CHARACTER(LEN=*), INTENT(IN) :: NAME
      TYPE(INDEXSTR), INTENT(IN) :: ISLIST(:)
      
      LOGICAL FIRST
      CHARACTER(LEN=15) :: INDSTR

      WRITE(AC%BUNIT,"(A,A)", ADVANCE="NO")NAME,'{'
      FIRST=.TRUE.
      DO I=1,SIZE(ISLIST)
         IF(.NOT.FIRST)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")", "
         WRITE(INDSTR,'(I15)')ISLIST(I)%INDEX
         WRITE(AC%BUNIT,"(A,A,A,A)", ADVANCE="NO")TRIM(ADJUSTL(INDSTR)), &
              ": '",TRIM(ISLIST(I)%STR),"'"
         FIRST=.FALSE.
      ENDDO
      WRITE(AC%BUNIT,"(A)", ADVANCE="NO")'}'
    END SUBROUTINE WRITESTRLIST

    SUBROUTINE WRITEINTLIST(NAME,ILIST)
      USE AUTO_TYPES, ONLY: INDEXVAR
      CHARACTER(LEN=*), INTENT(IN) :: NAME
      INTEGER, INTENT(IN) :: ILIST(:)
      
      LOGICAL FIRST
      CHARACTER(LEN=12) :: INDSTR

      IF (SIZE(ILIST) == 0) RETURN
      WRITE(AC%BUNIT,"(A,A)", ADVANCE="NO")NAME,'['
      FIRST=.TRUE.
      DO I=1,SIZE(ILIST)
         IF(.NOT.FIRST)WRITE(AC%BUNIT,"(A)", ADVANCE="NO")", "
         WRITE(INDSTR,'(I12)')ILIST(I)
         WRITE(AC%BUNIT,"(A)", ADVANCE="NO")TRIM(ADJUSTL(INDSTR))
         FIRST=.FALSE.
      ENDDO
      WRITE(AC%BUNIT,"(A)", ADVANCE="NO")']'
    END SUBROUTINE WRITEINTLIST

  END SUBROUTINE STHD

! ---------- ------
  SUBROUTINE HEADNG(AC,ICP,IUNIT,N1,N2)

    USE COMPAT

! Prints headings above columns on unit 6, 7, and 9.
! N1 = number of parameters to print (maximum: 7 for screen output)
! N2 = number of (max) variables to print (maximum: max(0,7-N1,7))

    TYPE(AUTOCONTEXT), INTENT(IN) :: AC
    INTEGER, INTENT(IN) :: ICP(*),IUNIT,N1,N2
! Local
    INTEGER I,J,IPS,IPLT,NDM
    CHARACTER(LEN=13) name

    IPS=AC%AP%IPS
    IPLT=AC%AP%IPLT
    NDM=AC%AP%NDM

    IF(IUNIT.EQ.AC%BUNIT)THEN
       WRITE(AC%BUNIT,"(I4/I4,A)",ADVANCE="NO")0,0,'    PT  TY  LAB '
    ELSE
       WRITE(IUNIT,"(/A)",ADVANCE="NO")'  BR    PT  TY  LAB '
    ENDIF

    DO J=1,N1+N2+1
       IF(J==1.OR.J>N2+2)THEN
          I=1
          IF(J>1)I=J-N2-1
          name = getname(AC%parnames, ICP(I))
          IF(LEN_TRIM(name)>0)THEN
             CALL WRITECOL(-1,name)
          ELSEIF(ICP(I)==11.AND.IPS>0.AND.IPS/=4.AND.IPS/=7)THEN
             CALL WRITECOL(5,'PERIOD')
          ELSEIF(ICP(I)==10.AND.(IPS==5.OR.IPS==15))THEN
             CALL WRITECOL(6,'FOPT')
          ELSEIF(ICP(I)==14.AND.(IPS==14.OR.IPS==16))THEN
             CALL WRITECOL(6,'TIME')
          ELSE
             CALL WRITECOL(4,'PAR',ICP(I))
          ENDIF
       ELSEIF(J==2.AND.(IPLT==0.OR.IPLT<-NDM.OR.IPLT>3*NDM))THEN
          CALL WRITECOL(4,'L2-NORM')
       ELSEIF(J==2)THEN
          name = getname(AC%unames, MOD(ABS(IPLT)-1,NDM)+1)
          IF(LEN_TRIM(name)>0)THEN
             IF(IPLT>NDM.AND.IPLT<=2*NDM) THEN
                CALL WRITECOL(-1, 'INTEGRAL ' // name)
             ELSE IF(IPLT>2*NDM.AND.IPLT<=3*NDM) THEN
                CALL WRITECOL(-1, 'L2-NORM '// name)
             ELSE IF(ABS(IPLT)<=NDM) THEN
                IF(ABS(IPS)<=1.OR.IPS==5.OR.IPS==11)THEN
                   CALL WRITECOL(2, name)
                ELSE IF(IPLT>0)THEN
                   CALL WRITECOL(-1, 'MAX ' // name)
                ELSE
                   CALL WRITECOL(-1, 'MIN ' // name)
                ENDIF
             ENDIF
          ELSE
             IF(IPLT>NDM.AND.IPLT<=2*NDM) THEN
                CALL WRITECOL(2,'INTEGRAL U',IPLT-NDM)
             ELSE IF(IPLT>2*NDM.AND.IPLT<=3*NDM) THEN
                CALL WRITECOL(2,'L2-NORM U',IPLT-2*NDM)
             ELSE IF(IPLT/=0.AND.ABS(IPLT)<=NDM) THEN
                IF(ABS(IPS)<=1.OR.IPS==5.OR.IPS==11)THEN
                   CALL WRITECOL(6,'U',ABS(IPLT))
                ELSE IF(IPLT>0)THEN
                   CALL WRITECOL(4,'MAX U',IPLT)
                ELSE
                   CALL WRITECOL(4,'MIN U',-IPLT)
                ENDIF
             ENDIF
          ENDIF
       ELSE !J>2 with N2>0
          name = getname(AC%unames, J-2)
          IF(LEN_TRIM(name)>0)THEN
             IF(ABS(IPS)<=1.OR.IPS==5.OR.IPS==11)THEN
                CALL WRITECOL(-1,name)
             ELSE
                CALL WRITECOL(-1,'MAX '//name)
             ENDIF
          ELSEIF(ABS(IPS)<=1.OR.IPS==5.OR.IPS==11)THEN
             CALL WRITECOL(6,'U',J-2)
          ELSE
             CALL WRITECOL(4,'MAX U',J-2)
          ENDIF
       ENDIF

    ENDDO

    WRITE(IUNIT,"()")
    CALL AUTOFLUSH(IUNIT)

  CONTAINS

    SUBROUTINE WRITECOL(II,S,N)
      INTEGER, INTENT(IN) :: II
      CHARACTER(*), INTENT(IN) :: S
      INTEGER, INTENT(IN), OPTIONAL :: N
! Local
      CHARACTER(10) SN
      CHARACTER(19) COL
      INTEGER I
      COL=' '
      I=II
      IF(I==-1)THEN
         ! centre into the column
         I=MAX(7-(LEN_TRIM(S)-1)/2,2)
      ENDIF
      IF(PRESENT(N))THEN
         WRITE(SN,"(I10)")N
         WRITE(COL(I:),"(A,A,A,A)") S,'(',TRIM(ADJUSTL(SN)),')'
      ELSE
         WRITE(COL(I:),"(A)") TRIM(S)
      ENDIF
      IF(IUNIT.EQ.AC%BUNIT)THEN
         WRITE(IUNIT,"(A19)",ADVANCE="NO")COL
      ELSE
         WRITE(IUNIT,"(A14)",ADVANCE="NO")COL
      ENDIF
    END SUBROUTINE WRITECOL
    
  END SUBROUTINE HEADNG

! ---------- ------
  SUBROUTINE WRLINE(AC,PAR,ICU,IBR,NTOT,LAB,VAXIS,U)

    USE COMPAT
    USE SUPPORT, ONLY: LBTYPE

! Write one line of output on unit 6 and 7.

    TYPE(AUTOCONTEXT), INTENT(IN) :: AC
    INTEGER, INTENT(IN) :: ICU(*),IBR,NTOT,LAB
    DOUBLE PRECISION, INTENT(IN) :: PAR(*),U(*),VAXIS
! Local
    CHARACTER(3) ATYPE
    CHARACTER(33) :: F69 ! (I4,I6,2X,A2,I5,**********ES14.5)
    CHARACTER(31) :: F7  ! (I4,I6,I4,I5,**********ES19.10)
    INTEGER MTOT,NDM,ITP,NICP,N1,N2,I

    NDM=AC%AP%NDM
    ITP=AC%AP%ITP
    NICP=AC%AP%NICP

    N1=NICP
    N2=NDM

    IF(N1.GT.7)THEN
       N1=7
       N2=0
    ELSEIF(N1+N2.GT.7)THEN
       N2=7-N1
    ENDIF

! Write a heading above the first line.

    IF(ABS(NTOT).EQ.1)CALL HEADNG(AC,ICU,6,N1,N2)
    IF(ABS(NTOT).EQ.1)CALL HEADNG(AC,ICU,AC%BUNIT,NICP,N2)
    IF(AC%AP%IID>0)CALL HEADNG(AC,ICU,AC%DUNIT,N1,N2)

    ATYPE=ADJUSTR(LBTYPE(ITP))

    IF(NTOT>0)THEN
       MTOT=MOD(NTOT-1,9999)+1
    ELSE
       MTOT=-MOD(-NTOT-1,9999)-1
    ENDIF
    WRITE(F69,"(A,I10,A)") '(I4,I6,1X,A3,I5,',N1+N2+1,'ES14.5)'
    WRITE(F7,"(A,I10,A)") '(I4,I6,I4,I5,',NICP+N2+1,'ES19.10)'
    IF(MOD(ITP,10).NE.0)THEN
       WRITE(6,F69)ABS(IBR),ABS(MTOT),ATYPE,LAB,PAR(ICU(1)),VAXIS, &
            (U(I),I=1,N2),(PAR(ICU(I)),I=2,N1)
       CALL AUTOFLUSH(6)
    ENDIF
    WRITE(AC%BUNIT,F7)IBR,MTOT,ITP,LAB,PAR(ICU(1)),VAXIS, &
         (U(I),I=1,N2),(PAR(ICU(I)),I=2,NICP)
    CALL SCGAUT(AC%ID,IBR,MTOT,ITP,LAB,U,PAR)
    CALL AUTOFLUSH(AC%BUNIT)
    IF(AC%AP%IID>0)WRITE(AC%DUNIT,F69)IBR,MTOT,ATYPE,LAB,PAR(ICU(1)),VAXIS, &
         (U(I),I=1,N2),(PAR(ICU(I)),I=2,N1)
  END SUBROUTINE WRLINE

! ---------- -----
  SUBROUTINE WRBAR(AC,C,N)

    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    CHARACTER(1), INTENT(IN) :: C
    INTEGER, INTENT(IN) :: N
    INTEGER I

    WRITE(AC%DUNIT,101)(C,I=1,N)
101 FORMAT(80A1)
  END SUBROUTINE WRBAR

! ---------- ------
  SUBROUTINE NEWLAB(AC)

! Determine a suitable label when restarting.


    TYPE(AUTOCONTEXT), INTENT(INOUT) :: AC

    INTEGER IPS,IRS,ISW,ITP
    INTEGER IBR

    IPS=AC%AP%IPS
    IRS=AC%AP%IRS
    ISW=AC%AP%ISW
    ITP=AC%AP%ITP
    
    IBR=AC%IBR

    IF(AC%AP%LAB.EQ.0)AC%AP%LAB=AC%IO%MLAB+1
    IF(IBR.NE.0)RETURN
    IF(ISW.LT.0.OR.IRS.EQ.0)THEN
       AC%AP%IBR=AC%IO%MBR+1
    ELSEIF( (ABS(ITP).LT.10.AND.ABS(ISW).EQ.2) &
         .OR. ((IPS.EQ.2.OR.IPS.EQ.12).AND.ITP.EQ.3) &
         .OR. (IPS.EQ.4.AND.ISW.EQ.2.AND.ABS(ITP).LT.10) &
         .OR. (IPS.EQ.5.AND.MOD(ITP,10).EQ.2) )THEN
       AC%AP%IBR=IRS
    ENDIF

  END SUBROUTINE NEWLAB

! ------- -------- --------
  INTEGER FUNCTION GETNDIM3(AC)
    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    GETNDIM3 = AC%IO%CURSOL%NAR-1
  END FUNCTION GETNDIM3

! ------- -------- --------
  INTEGER FUNCTION GETNTST3(AC)
    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    GETNTST3 = AC%IO%CURSOL%NTST
  END FUNCTION GETNTST3

! ------- -------- --------
  INTEGER FUNCTION GETNCOL3(AC)
    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    GETNCOL3 = AC%IO%CURSOL%NCOL
  END FUNCTION GETNCOL3

! ------- -------- --------
  INTEGER FUNCTION GETNFPR3(AC)
    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    GETNFPR3 = AC%IO%CURSOL%NFPR
  END FUNCTION GETNFPR3

! ------- -------- --------
  INTEGER FUNCTION GETIPS3(AC)
    TYPE(AUTOCONTEXT), INTENT(IN)::AC
    GETIPS3 = AC%IO%CURSOL%IPS
  END FUNCTION GETIPS3

! ---------- ------
  SUBROUTINE FINDLB(AC,UNITC,IRS,NFPR,NPAR,FOUND)

    USE SUPPORT, ONLY: LBTYPE, AUTOSTOP

    TYPE(AUTOCONTEXT), INTENT(INOUT) :: AC
    INTEGER, INTENT(IN) :: UNITC
    INTEGER, INTENT(INOUT) :: IRS
    INTEGER, INTENT(OUT) :: NFPR,NPAR
    LOGICAL, INTENT(OUT) :: FOUND

    LOGICAL EOF3
    INTEGER IBR,NTOT,ITP,LAB,NFPRR,ISWR,NTPL,NAR,NROWPR,NTST,NCOL,NPARR
    INTEGER NPARI,NDM,IPS,IPRIV
    INTEGER ISW,ITPST,I,J,ios,number,UNIT
    CHARACTER(3) :: ATYPE
    CHARACTER(100) :: HEADER

! Locates restart point with label IRS and determines type.
! If the label can not be located on unit 3 then FOUND will be .FALSE.
! If IRS is negative, then just pick the abs(IRS)th solution.

    FOUND=.FALSE.
    NFPR=0
    NPAR=0
    ISW=AC%AP%ISW

    UNIT=AC%SUNIT
    ios=0
    IF(LEN_TRIM(AC%SFILE)==0)THEN
       OPEN(UNIT,FILE='fort.3',STATUS='old',ACCESS='sequential',IOSTAT=ios)
    ELSEIF(TRIM(AC%SFILE)=='/')THEN
       AC%SFILE=''
       UNIT=UNITC
    ELSE
       OPEN(UNIT,FILE='s.'//AC%SFILE,STATUS='old',ACCESS='sequential',IOSTAT=ios)
    ENDIF
    IF(ios/=0)THEN
       WRITE(6,'(A,A)')'The solution file (fort.3 or s. file) ',&
            'could not be found.'
       CALL AUTOSTOP()
    ENDIF
    I=0
    J=SCAN(AC%SIRS,"-0123456789")
    number=0
    IF(J>2)THEN
       ATYPE=AC%SIRS(1:J-1)
       READ(AC%SIRS(J:),'(I11)',IOSTAT=ios)number
       IF(ios/=0)THEN
          number=0
       ENDIF
    ENDIF
    DO
       I=I+1
       READ(UNIT,'(A)',END=2)HEADER
       IF(LEN_TRIM(HEADER) <= 73)THEN
          READ(HEADER,*)IBR,NTOT,ITP,LAB,NFPRR,ISWR,NTPL,NAR,NROWPR,NTST, &
               NCOL,NPARR
          NPARI=0
          NDM=0
          IPS=0
          IPRIV=0
       ELSE
          READ(HEADER,*)IBR,NTOT,ITP,LAB,NFPRR,ISWR,NTPL,NAR,NROWPR,NTST, &
               NCOL,NPARR,NPARI,NDM,IPS,IPRIV
       ENDIF
       IF(IBR>AC%IO%MBR)AC%IO%MBR=IBR
       IF(LAB>AC%IO%MLAB)AC%IO%MLAB=LAB
       IF(number>0.AND.ATYPE==LBTYPE(ITP))THEN
          number=number-1
          IF(number==0)IRS=LAB
       ENDIF

       IF((LAB==IRS.OR.IRS==-I).AND..NOT.FOUND.AND.number==0)THEN
          IRS=LAB
          NFPR=NFPRR
          NPAR=NPARR
          FOUND=.TRUE.
          IF(AC%AP%ITP==0)AC%AP%ITP=ITP
          IF(AC%AP%IBR==0)AC%AP%IBR=IBR
          IF(ABS(ISW).GE.2)THEN
             IF(ABS(ITP).LT.10)THEN
                ITPST=ABS(ITP)
             ELSE
                ITPST=ABS(ITP/10)
             ENDIF
          ELSE
             ITPST=0
          ENDIF
          AC%AP%ITPST=ITPST
          CALL READSOL(AC,UNIT,IBR,NTOT,ITP,LAB,NFPR,ISWR,NTPL,NAR,NROWPR,NTST,&
               NCOL,NPAR,NPARI,NDM,IPS,IPRIV)
          ! strip internal parameters from returned NPAR so they can
          ! be thrown away when possible
          NPAR=NPAR-NPARI
       ELSE
          CALL SKIPS(UNIT,NROWPR,EOF3)
          IF(EOF3)GOTO 2
       ENDIF
    ENDDO

2   CONTINUE
    CLOSE(UNIT)

  END SUBROUTINE FINDLB

! ---------- -------
  SUBROUTINE READSOL(AC,UNIT,IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NROWPR,NTST,NCOL,&
       NPAR,NPARI,NDM,IPS,IPRIV)


    TYPE(AUTOCONTEXT), INTENT(INOUT)::AC
    INTEGER, INTENT(IN) :: UNIT,IBR,NTOT,ITP,LAB,NFPR
    INTEGER, INTENT(IN) :: ISW,NTPL,NAR,NROWPR,NTST,NCOL,NPAR
    INTEGER, INTENT(IN) :: NPARI,NDM,IPS,IPRIV

! Local
    INTEGER J, NTNC, NROWPRSMALL

    NULLIFY(AC%IO%ROOTSOL)
    NTNC=NTPL-1

    CALL NEWSOL(AC,IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NROWPR,NTST,NCOL,NPAR,&
         NPARI,NDM,IPS,IPRIV)
    DO J=0,NTNC
       READ(UNIT,*)AC%IO%CURSOL%TM(J),AC%IO%CURSOL%UPS(:,J)
    ENDDO

    NROWPRSMALL=((NAR-1)/7+1)*NTPL + (NPAR+6)/7
    IF(NTST>0.AND.NROWPR>NROWPRSMALL)THEN
       READ(UNIT,*)AC%IO%CURSOL%ICP(:)
       READ(UNIT,*)AC%IO%CURSOL%RLDOT(:)

! Read U-dot (derivative with respect to arclength).

       READ(UNIT,*)AC%IO%CURSOL%UDOTPS(:,:)
    ENDIF

! Read the parameter values.

    READ(UNIT,*)AC%IO%CURSOL%PAR(:)

  END SUBROUTINE READSOL

! ---------- ------
  SUBROUTINE NEWSOL(AC,IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NROWPR,NTST,NCOL,NPAR,&
       NPARI,NDM,IPS,IPRIV)

    TYPE(AUTOCONTEXT), INTENT(INOUT)::AC
    INTEGER, INTENT(IN) :: IBR,NTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NROWPR,&
         NTST,NCOL,NPAR,NPARI,NDM,IPS,IPRIV
    TYPE(SOLUTION), POINTER :: SOL
    INTEGER NDIM,NTNC

    ALLOCATE(SOL)
    SOL%IBR = IBR
    SOL%NTOT = NTOT
    SOL%ITP = ITP
    SOL%LAB = LAB
    SOL%NFPR = NFPR
    SOL%ISW = ISW
    SOL%NTPL = NTPL
    SOL%NAR = NAR
    SOL%NROWPR = NROWPR
    SOL%NTST = NTST
    SOL%NCOL = NCOL
    SOL%NPAR = NPAR
    SOL%NPARI = NPARI
    SOL%NDM = NDM
    SOL%IPS = IPS
    SOL%IPRIV = IPRIV
    NDIM=NAR-1
    NTNC=NTPL-1
    ALLOCATE(SOL%UPS(NDIM,0:NTNC),SOL%TM(0:NTNC),SOL%PAR(NPAR))
    IF(NTST>0)THEN
       ALLOCATE(SOL%UDOTPS(NDIM,0:NTNC),SOL%ICP(NFPR),SOL%RLDOT(NFPR))
    ELSE
       NULLIFY(SOL%UDOTPS,SOL%ICP,SOL%RLDOT)
    ENDIF
    NULLIFY(SOL%NEXT)
    IF(ASSOCIATED(AC%IO%ROOTSOL))THEN
       AC%IO%CURSOL%NEXT => SOL
    ELSE
       AC%IO%ROOTSOL => SOL
    ENDIF
    AC%IO%CURSOL => SOL
  END SUBROUTINE NEWSOL

! ---------- ------
  SUBROUTINE READLB(AC,ICPRS,U,UDOT,PAR)

    USE SUPPORT, ONLY: NAMEIDX
    TYPE(AUTOCONTEXT), INTENT(IN) :: AC
    INTEGER, INTENT(OUT) :: ICPRS(*)
    DOUBLE PRECISION, INTENT(OUT) :: U(*),UDOT(*),PAR(*)

    DOUBLE PRECISION, ALLOCATABLE :: UX(:),P(:)
    INTEGER I,NFPR,NPARR,NPAR,NDIM,NDM,NDMRD

    NPAR=AC%AP%NPAR
    NDM=AC%AP%NDM
    IF(AC%AP%IPS==11)NDM=NDM/2

! Reads the restart data for algebraic problems.

    NFPR=AC%IO%CURSOL%NFPR
    NPARR=AC%IO%CURSOL%NPAR
    NDIM=MIN(AC%IO%CURSOL%NAR-1,AC%AP%NDIM)
    U(1:NDIM)=AC%IO%CURSOL%UPS(1:NDIM,0)
    IF(AC%IO%CURSOL%NTST>0)THEN
       ICPRS(1:NFPR)=AC%IO%CURSOL%ICP(1:NFPR)
       UDOT(NDIM+1)=AC%IO%CURSOL%RLDOT(1)
       UDOT(NDIM-NFPR+2:NDIM)=AC%IO%CURSOL%RLDOT(2:NFPR)
       UDOT(1:NDIM)=AC%IO%CURSOL%UDOTPS(1:NDIM,0)
    ENDIF
    PAR(1:NPARR)=AC%IO%CURSOL%PAR(1:NPARR)

! override parameter/point values with values from constants file

    DO I=1,SIZE(AC%UVALS)
       U(NAMEIDX(AC%UVALS(I)%INDEX,AC%unames))=AC%UVALS(I)%VAR
    ENDDO
    DO I=1,SIZE(AC%PARVALS)
       PAR(NAMEIDX(AC%PARVALS(I)%INDEX,AC%parnames))=AC%PARVALS(I)%VAR
    ENDDO

    NDMRD=AC%IO%CURSOL%NDM
    IF(NDMRD==0)THEN
       ! old-style solution file: try to get NDM: this works for
       ! equilibria/maps/timeint problems (not parabolic/optimization)
       NDMRD=AC%IO%CURSOL%NAR-1
       IF(ABS(AC%IO%CURSOL%ISW)==2)THEN
          IF(AC%IO%CURSOL%ITP/10==3)THEN
             NDMRD=NDMRD/3 ! HB (old method)
          ELSE
             NDMRD=(NDMRD-1)/2 ! PD/BP/LP/TR
          ENDIF
       ENDIF
    ENDIF
    IF(NDM>NDMRD)THEN
       ! system is extended; call STPNT for extension
       ALLOCATE(UX(NDM),P(NPAR))
       P(:)=PAR(:NPAR)
       UX(1:NDMRD)=U(1:NDMRD)
       UX(NDMRD+1:NDM)=0.d0
       CALL STPNT(AC%ID,NDM,UX,P,0d0)
       U(NDMRD+1:NDM)=UX(NDMRD+1:NDM)
       DEALLOCATE(UX,P)
    ENDIF

  END SUBROUTINE READLB

! ---------- ------
  SUBROUTINE READBV(AC,PAR,ICPRS,NTSRS,NCOLRS,NDIMRD,RLDOTRS,UPS, &
       UDOTPS,TM,ITPRS,NDIM)

    USE SUPPORT, ONLY: NAMEIDX
    INTEGER, INTENT(IN) :: NDIM
    TYPE(AUTOCONTEXT), INTENT(IN) :: AC
    INTEGER, INTENT(OUT) :: ICPRS(*),NTSRS,NCOLRS,NDIMRD,ITPRS
    DOUBLE PRECISION, INTENT(OUT) :: RLDOTRS(*),UPS(NDIM,0:*),UDOTPS(NDIM,0:*)
    DOUBLE PRECISION, INTENT(OUT) :: TM(0:*),PAR(*)
! Local
    INTEGER I,J,N,NFPR,NFPRS,NPARR,NPARIR,NPAR,NPARI,NDM,NDMRD
    DOUBLE PRECISION, ALLOCATABLE :: U(:),P(:)

    NPARI=AC%AP%NPARI
    NFPR=AC%AP%NFPR
    NPAR=AC%AP%NPAR
    NDM=AC%AP%NDM
    IF(AC%AP%IPS==12)NDM=NDM/2
    ITPRS=AC%IO%CURSOL%ITP
    NFPRS=AC%IO%CURSOL%NFPR
    NTSRS=AC%IO%CURSOL%NTST
    NCOLRS=AC%IO%CURSOL%NCOL
    NPARR=AC%IO%CURSOL%NPAR
    NPARIR=AC%IO%CURSOL%NPARI

    NDIMRD=MIN(NDIM,AC%IO%CURSOL%NAR-1)

    DO J=0,NTSRS*NCOLRS
       IF(MOD(J,NCOLRS)==0)THEN
          TM(J/NCOLRS)=AC%IO%CURSOL%TM(J)
       ENDIF
       UPS(1:NDIMRD,J)=AC%IO%CURSOL%UPS(1:NDIMRD,J)
    ENDDO

    ICPRS(1:MIN(NFPR,NFPRS))=AC%IO%CURSOL%ICP(1:MIN(NFPR,NFPRS))
    RLDOTRS(1:MIN(NFPR,NFPRS))=AC%IO%CURSOL%RLDOT(1:MIN(NFPR,NFPRS))

! Read U-dot (derivative with respect to arclength).

    DO J=0,NTSRS*NCOLRS
       UDOTPS(1:NDIMRD,J)=AC%IO%CURSOL%UDOTPS(1:NDIMRD,J)
    ENDDO

! Read the internal parameter values
 
    IF(NPARI>0)THEN
       IF(NPARIR>0)THEN
          N=MIN(NPARIR,NPARI)
          PAR(NPAR-NPARI+1:NPAR-NPARI+N)= &
               AC%IO%CURSOL%PAR(NPARR-NPARIR+1:NPARR-NPARIR+N)
       ELSEIF(AC%IO%CURSOL%NDM==0)THEN ! old style solution file, copy from PAR(12:)
          N=MIN(NPARR-11,NPARI)
          IF(N>0)THEN
             PAR(NPAR-NPARI+1:NPAR-NPARI+N)=AC%IO%CURSOL%PAR(12:11+N)
          ENDIF
       ENDIF
    ENDIF

! Read the other parameter values.

    PAR(1:NPARR-NPARIR)=AC%IO%CURSOL%PAR(1:NPARR-NPARIR)

! override parameter values with values from constants file

    DO I=1,SIZE(AC%PARVALS)
       PAR(NAMEIDX(AC%PARVALS(I)%INDEX,AC%parnames))=AC%PARVALS(I)%VAR
    ENDDO

    NDMRD=AC%IO%CURSOL%NDM
    IF(NDMRD==0)THEN
       ! old-style solution file: try to get NDM: this works for
       ! periodic/general BVP problems (not HomCont/parabolic/optimization)
       NDMRD=AC%IO%CURSOL%NAR-1
       IF(ABS(AC%IO%CURSOL%ISW)==2)THEN
          IF(AC%IO%CURSOL%ITP/10==8)THEN
             NDMRD=NDMRD/3 ! TR
          ELSEIF(AC%IO%CURSOL%ITP/10==6.AND.AC%IO%CURSOL%ISW==-2)THEN
             NDMRD=NDMRD/4 ! BP start
          ELSE
             NDMRD=NDMRD/2 ! PD/BP/LP
          ENDIF
       ENDIF
    ENDIF
    IF(NDM>NDMRD)THEN
       ! system is extended; call STPNT for extension
       ALLOCATE(U(NDM),P(NPAR))
       P(:)=PAR(:NPAR)
       U(NDMRD+1:NDM)=0.d0
       DO J=0,NTSRS*NCOLRS
          U(1:NDMRD)=UPS(1:NDMRD,J)
          CALL STPNT(AC%ID,NDM,U,P,AC%IO%CURSOL%TM(J))
          UPS(NDMRD+1:NDM,J)=U(NDMRD+1:NDM)
       ENDDO
       DEALLOCATE(U,P)
    ENDIF

  END SUBROUTINE READBV

! ---------- -----
  SUBROUTINE SKIPS(UNIT,NSKIP,EOF3)

! Skips the specified number of lines on the solution file unit

    INTEGER, INTENT(IN) :: UNIT,NSKIP
    LOGICAL, INTENT(OUT) :: EOF3
    CHARACTER(12) FMT
    INTEGER I

    EOF3=.TRUE.
    IF(NSKIP<=1)THEN
       DO I=1,NSKIP
         READ(UNIT,'(A)',END=2)
       ENDDO
    ELSE
       WRITE(FMT,'(A,I9,A)')'(',NSKIP-1,'/)'
       READ(UNIT,FMT,END=2)
    ENDIF
    EOF3=.FALSE.
2   RETURN

  END SUBROUTINE SKIPS

END MODULE IO
