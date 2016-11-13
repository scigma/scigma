!
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!        Subroutines for Time Integration of ODEs
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------

MODULE TIMEINT

  USE AUTO_TYPES, ONLY: AUTOCONTEXT, AUTOPARAMETERS
  USE AE
  USE TOOLBOXAE
  USE INTERFACES

  IMPLICIT NONE
  PRIVATE
  PUBLIC :: AUTOTI

CONTAINS

! ---------- ------
  SUBROUTINE AUTOTI(AC,ICP,ICU)

    TYPE(AUTOCONTEXT), INTENT(INOUT), TARGET :: AC
    TYPE(AUTOPARAMETERS), POINTER :: AP
    INTEGER, INTENT(INOUT) :: ICP(:)
    INTEGER, INTENT(IN) :: ICU(:)

    ! ** Time integration (IPS==-2)
    AP=>AC%AP
    
    AP%NFPR=1
    AP%ISP=0
    AP%ILP=0
    ICP(1)=14
    CALL AUTOAE(AC,ICP,ICU,FNTI,STPNAE,FNCSAE)
  END SUBROUTINE AUTOTI

! ---------- ----
  SUBROUTINE FNTI(AC,NDIM,U,UOLD,ICP,PAR,IJAC,F,DFDU,DFDP)

    ! Generate the equations for time integration.

    TYPE(AUTOCONTEXT), INTENT(IN), TARGET :: AC
    INTEGER, INTENT(IN) :: ICP(*),NDIM,IJAC
    DOUBLE PRECISION, INTENT(IN) :: UOLD(*)
    DOUBLE PRECISION, INTENT(INOUT) :: U(NDIM),PAR(*)
    DOUBLE PRECISION, INTENT(OUT) :: F(NDIM)
    DOUBLE PRECISION, INTENT(INOUT) :: DFDU(NDIM,NDIM),DFDP(NDIM,*)

    INTEGER I,J
    DOUBLE PRECISION TOLD,DT

    CALL FUNI(AC,NDIM,U,UOLD,ICP,PAR,IJAC,F,DFDU,DFDP)

    TOLD=UOLD(NDIM+1)
    DT=PAR(ICP(1))-TOLD

    DO I=1,NDIM
       DFDP(I,ICP(1))=F(I)
       F(I)= DT*F(I) - U(I) + UOLD(I)
    ENDDO

    IF(IJAC.EQ.0)RETURN

    DO I=1,NDIM
       DO J=1,NDIM
          DFDU(I,J)= DT*DFDU(I,J)
       ENDDO
       DFDU(I,I)= DFDU(I,I) - 1.d0
    ENDDO
  END SUBROUTINE FNTI

END MODULE TIMEINT
