      SUBROUTINE DGESVJ( JOBA, JOBU, JOBV, M, N, A, LDA, SVA, MV, V,
     +                   LDV, WORK, LWORK, INFO )

      IMPLICIT           NONE

      INTEGER            INFO, LDA, LDV, LWORK, M, MV, N
      CHARACTER*1        JOBA, JOBU, JOBV

      DOUBLE PRECISION   A( LDA, * ), SVA( N ), V( LDV, * ),
     +                   WORK( LWORK )

      DOUBLE PRECISION   ZERO, HALF, ONE, TWO
      PARAMETER          ( ZERO = 0.0D0, HALF = 0.5D0, ONE = 1.0D0,
     +                   TWO = 2.0D0 )
      INTEGER            NSWEEP
      PARAMETER          ( NSWEEP = 30 )

      DOUBLE PRECISION   AAPP, AAPP0, AAPQ, AAQQ, APOAQ, AQOAP, BIG,
     +                   BIGTHETA, CS, CTOL, EPSILON, LARGE, MXAAPQ,
     +                   MXSINJ, ROOTBIG, ROOTEPS, ROOTSFMIN, ROOTTOL,
     +                   SCALE, SFMIN, SMALL, SN, T, TEMP1, THETA,
     +                   THSIGN, TOL
      INTEGER            BLSKIP, EMPTSW, i, ibr, IERR, igl, IJBLSK, ir1,
     +                   ISWROT, jbc, jgl, KBL, LKAHEAD, MVL, N2, N34,
     +                   N4, NBL, NOTROT, p, PSKIPPED, q, ROWSKIP,
     +                   SWBAND
      LOGICAL            APPLV, GOSCALE, LOWER, LSVEC, NOSCALE, ROTOK,
     +                   RSVEC, UCTOL, UPPER

      DOUBLE PRECISION   FASTR( 5 )

      INTRINSIC          DABS, DMAX1, DMIN1, DBLE, MIN0, DSIGN, DSQRT

      DOUBLE PRECISION   DDOT, DNRM2
      EXTERNAL           DDOT, DNRM2
      INTEGER            IDAMAX
      EXTERNAL           IDAMAX

      DOUBLE PRECISION   DLAMCH
      EXTERNAL           DLAMCH
      LOGICAL            LSAME
      EXTERNAL           LSAME

      EXTERNAL           DAXPY, DCOPY, DROTM, DSCAL, DSWAP

      EXTERNAL           DLASCL, DLASET, DLASSQ, XERBLA

      EXTERNAL           DGSVJ0, DGSVJ1
*
      LSVEC = LSAME( JOBU, 'U' )
      UCTOL = LSAME( JOBU, 'C' )
      RSVEC = LSAME( JOBV, 'V' )
      APPLV = LSAME( JOBV, 'A' )
      UPPER = LSAME( JOBA, 'U' )
      LOWER = LSAME( JOBA, 'L' )
*
      IF( .NOT.( UPPER .OR. LOWER .OR. LSAME( JOBA, 'G' ) ) ) THEN
         INFO = -1
      ELSE IF( .NOT.( LSVEC .OR. UCTOL .OR. LSAME( JOBU, 'N' ) ) ) THEN
         INFO = -2
      ELSE IF( .NOT.( RSVEC .OR. APPLV .OR. LSAME( JOBV, 'N' ) ) ) THEN
         INFO = -3
      ELSE IF( M.LT.0 ) THEN
         INFO = -4
      ELSE IF( ( N.LT.0 ) .OR. ( N.GT.M ) ) THEN
         INFO = -5
      ELSE IF( LDA.LT.M ) THEN
         INFO = -7
      ELSE IF( MV.LT.0 ) THEN
         INFO = -9
      ELSE IF( ( RSVEC .AND. ( LDV.LT.N ) ) .OR.
     +         ( APPLV .AND. ( LDV.LT.MV ) ) ) THEN
         INFO = -11
      ELSE IF( UCTOL .AND. ( WORK( 1 ).LE.ONE ) ) THEN
         INFO = -12
      ELSE IF( LWORK.LT.MAX0( M+N, 6 ) ) THEN
         INFO = -13
      ELSE
         INFO = 0
      END IF
*
*     #:(
      IF( INFO.NE.0 ) THEN
         CALL XERBLA( 'DGESVJ', -INFO )
         RETURN
      END IF
*
* #:) Quick return for void matrix
*
      IF( ( M.EQ.0 ) .OR. ( N.EQ.0 ) )RETURN
*
*     Set numerical parameters
*     The stopping criterion for Jacobi rotations is
*
*     max_{i<>j}|A(:,i)^T * A(:,j)|/(||A(:,i)||*||A(:,j)||) < CTOL*EPS
*
*     where EPS is the round-off and CTOL is defined as follows:
*
      IF( UCTOL ) THEN
*        ... user controlled
         CTOL = WORK( 1 )
      ELSE
*        ... default
         IF( LSVEC .OR. RSVEC .OR. APPLV ) THEN
            CTOL = DSQRT( DBLE( M ) )
         ELSE
            CTOL = DBLE( M )
         END IF
      END IF
*     ... and the machine dependent parameters are
*[!]  (Make sure that DLAMCH() works properly on the target machine.)
*
      EPSILON = DLAMCH( 'Epsilon' )
      ROOTEPS = DSQRT( EPSILON )
      SFMIN = DLAMCH( 'SafeMinimum' )
      ROOTSFMIN = DSQRT( SFMIN )
      SMALL = SFMIN / EPSILON
      BIG = DLAMCH( 'Overflow' )
*     BIG         = ONE    / SFMIN
      ROOTBIG = ONE / ROOTSFMIN
      LARGE = BIG / DSQRT( DBLE( M*N ) )
      BIGTHETA = ONE / ROOTEPS
*
      TOL = CTOL*EPSILON
      ROOTTOL = DSQRT( TOL )
*
      IF( DBLE( M )*EPSILON.GE.ONE ) THEN
         INFO = -5
         CALL XERBLA( 'DGESVJ', -INFO )
         RETURN
      END IF
*
*     Initialize the right singular vector matrix.
*
      IF( RSVEC ) THEN
         MVL = N
         CALL DLASET( 'A', MVL, N, ZERO, ONE, V, LDV )
      ELSE IF( APPLV ) THEN
         MVL = MV
      END IF
      RSVEC = RSVEC .OR. APPLV
*
*     Initialize SVA( 1:N ) = ( ||A e_i||_2, i = 1:N )
*(!)  If necessary, scale A to protect the largest singular value
*     from overflow. It is possible that saving the largest singular
*     value destroys the information about the small ones.
*     This initial scaling is almost minimal in the sense that the
*     goal is to make sure that no column norm overflows, and that
*     DSQRT(N)*max_i SVA(i) does not overflow. If INFinite entries
*     in A are detected, the procedure returns with INFO=-6.
*
      SCALE = ONE / DSQRT( DBLE( M )*DBLE( N ) )
      NOSCALE = .TRUE.
      GOSCALE = .TRUE.
*
      IF( LOWER ) THEN
*        the input matrix is M-by-N lower triangular (trapezoidal)
         DO 1874 p = 1, N
            AAPP = ZERO
            AAQQ = ZERO
            CALL DLASSQ( M-p+1, A( p, p ), 1, AAPP, AAQQ )
            IF( AAPP.GT.BIG ) THEN
               INFO = -6
               CALL XERBLA( 'DGESVJ', -INFO )
               RETURN
            END IF
            AAQQ = DSQRT( AAQQ )
            IF( ( AAQQ.NE.ZERO ) .AND. NOSCALE ) THEN
               IF( AAPP.GE.( BIG / AAQQ ) ) THEN
                  NOSCALE = .FALSE.
               END IF
            END IF
            IF( NOSCALE ) THEN
               SVA( p ) = AAPP*AAQQ
            ELSE
               SVA( p ) = AAPP*( AAQQ*SCALE )
               IF( GOSCALE ) THEN
                  GOSCALE = .FALSE.
                  DO 1873 q = 1, p - 1
                     SVA( q ) = SVA( q )*SCALE
 1873             CONTINUE
               END IF
            END IF
 1874    CONTINUE
      ELSE IF( UPPER ) THEN
*        the input matrix is M-by-N upper triangular (trapezoidal)
         DO 2874 p = 1, N
            AAPP = ZERO
            AAQQ = ZERO
            CALL DLASSQ( p, A( 1, p ), 1, AAPP, AAQQ )
            IF( AAPP.GT.BIG ) THEN
               INFO = -6
               CALL XERBLA( 'DGESVJ', -INFO )
               RETURN
            END IF
            AAQQ = DSQRT( AAQQ )
            IF( ( AAQQ.NE.ZERO ) .AND. NOSCALE ) THEN
               IF( AAPP.GE.( BIG / AAQQ ) ) THEN
                  NOSCALE = .FALSE.
               END IF
            END IF
            IF( NOSCALE ) THEN
               SVA( p ) = AAPP*AAQQ
            ELSE
               SVA( p ) = AAPP*( AAQQ*SCALE )
               IF( GOSCALE ) THEN
                  GOSCALE = .FALSE.
                  DO 2873 q = 1, p - 1
                     SVA( q ) = SVA( q )*SCALE
 2873             CONTINUE
               END IF
            END IF
 2874    CONTINUE
      ELSE
*        the input matrix is M-by-N general dense
         DO 3874 p = 1, N
            AAPP = ZERO
            AAQQ = ZERO
            CALL DLASSQ( M, A( 1, p ), 1, AAPP, AAQQ )
            IF( AAPP.GT.BIG ) THEN
               INFO = -6
               CALL XERBLA( 'DGESVJ', -INFO )
               RETURN
            END IF
            AAQQ = DSQRT( AAQQ )
            IF( ( AAQQ.NE.ZERO ) .AND. NOSCALE ) THEN
               IF( AAPP.GE.( BIG / AAQQ ) ) THEN
                  NOSCALE = .FALSE.
               END IF
            END IF
            IF( NOSCALE ) THEN
               SVA( p ) = AAPP*AAQQ
            ELSE
               SVA( p ) = AAPP*( AAQQ*SCALE )
               IF( GOSCALE ) THEN
                  GOSCALE = .FALSE.
                  DO 3873 q = 1, p - 1
                     SVA( q ) = SVA( q )*SCALE
 3873             CONTINUE
               END IF
            END IF
 3874    CONTINUE
      END IF
*
      IF( NOSCALE )SCALE = ONE
*
*     Move the smaller part of the spectrum from the underflow threshold
*(!)  Start by determining the position of the nonzero entries of the
*     array SVA() relative to ( SFMIN, BIG ).
*
      AAPP = ZERO
      AAQQ = BIG
      DO 4781 p = 1, N
         IF( SVA( p ).NE.ZERO )AAQQ = DMIN1( AAQQ, SVA( p ) )
         AAPP = DMAX1( AAPP, SVA( p ) )
 4781 CONTINUE
*
* #:) Quick return for zero matrix
*
      IF( AAPP.EQ.ZERO ) THEN
         IF( LSVEC )CALL DLASET( 'G', M, N, ZERO, ONE, A, LDA )
         WORK( 1 ) = ONE
         WORK( 2 ) = ZERO
         WORK( 3 ) = ZERO
         WORK( 4 ) = ZERO
         WORK( 5 ) = ZERO
         WORK( 6 ) = ZERO
         RETURN
      END IF
*
* #:) Quick return for one-column matrix
*
      IF( N.EQ.1 ) THEN
         IF( LSVEC )CALL DLASCL( 'G', 0, 0, SVA( 1 ), SCALE, M, 1,
     +                           A( 1, 1 ), LDA, IERR )
         WORK( 1 ) = ONE / SCALE
         IF( SVA( 1 ).GE.SFMIN ) THEN
            WORK( 2 ) = ONE
         ELSE
            WORK( 2 ) = ZERO
         END IF
         WORK( 3 ) = ZERO
         WORK( 4 ) = ZERO
         WORK( 5 ) = ZERO
         WORK( 6 ) = ZERO
         RETURN
      END IF
*
*     Protect small singular values from underflow, and try to
*     avoid underflows/overflows in computing Jacobi rotations.
*
      SN = DSQRT( SFMIN / EPSILON )
      TEMP1 = DSQRT( BIG / DBLE( N ) )
      IF( ( AAPP.LE.SN ) .OR. ( AAQQ.GE.TEMP1 ) .OR.
     +    ( ( SN.LE.AAQQ ) .AND. ( AAPP.LE.TEMP1 ) ) ) THEN
         TEMP1 = DMIN1( BIG, TEMP1 / AAPP )
*         AAQQ  = AAQQ*TEMP1
*         AAPP  = AAPP*TEMP1
      ELSE IF( ( AAQQ.LE.SN ) .AND. ( AAPP.LE.TEMP1 ) ) THEN
         TEMP1 = DMIN1( SN / AAQQ, BIG / ( AAPP*DSQRT( DBLE( N ) ) ) )
*         AAQQ  = AAQQ*TEMP1
*         AAPP  = AAPP*TEMP1
      ELSE IF( ( AAQQ.GE.SN ) .AND. ( AAPP.GE.TEMP1 ) ) THEN
         TEMP1 = DMAX1( SN / AAQQ, TEMP1 / AAPP )
*         AAQQ  = AAQQ*TEMP1
*         AAPP  = AAPP*TEMP1
      ELSE IF( ( AAQQ.LE.SN ) .AND. ( AAPP.GE.TEMP1 ) ) THEN
         TEMP1 = DMIN1( SN / AAQQ, BIG / ( DSQRT( DBLE( N ) )*AAPP ) )
*         AAQQ  = AAQQ*TEMP1
*         AAPP  = AAPP*TEMP1
      ELSE
         TEMP1 = ONE
      END IF
*
*     Scale, if necessary
*
      IF( TEMP1.NE.ONE ) THEN
         CALL DLASCL( 'G', 0, 0, ONE, TEMP1, N, 1, SVA, N, IERR )
      END IF
      SCALE = TEMP1*SCALE
      IF( SCALE.NE.ONE ) THEN
         CALL DLASCL( JOBA, 0, 0, ONE, SCALE, M, N, A, LDA, IERR )
         SCALE = ONE / SCALE
      END IF
*
*     Row-cyclic Jacobi SVD algorithm with column pivoting
*
      EMPTSW = ( N*( N-1 ) ) / 2
      NOTROT = 0
      FASTR( 1 ) = ZERO
*
*     A is represented in factored form A = A * diag(WORK), where diag(WORK)
*     is initialized to identity. WORK is updated during fast scaled
*     rotations.
*
      DO 1868 q = 1, N
         WORK( q ) = ONE
 1868 CONTINUE
*
*
      SWBAND = 3
*[TP] SWBAND is a tuning parameter [TP]. It is meaningful and effective
*     if DGESVJ is used as a computational routine in the preconditioned
*     Jacobi SVD algorithm DGESVJ. For sweeps i=1:SWBAND the procedure
*     works on pivots inside a band-like region around the diagonal.
*     The boundaries are determined dynamically, based on the number of
*     pivots above a threshold.
*
      KBL = MIN0( 8, N )
*[TP] KBL is a tuning parameter that defines the tile size in the
*     tiling of the p-q loops of pivot pairs. In general, an optimal
*     value of KBL depends on the matrix dimensions and on the
*     parameters of the computer's memory.
*
      NBL = N / KBL
      IF( ( NBL*KBL ).NE.N )NBL = NBL + 1
*
      BLSKIP = KBL**2
*[TP] BLKSKIP is a tuning parameter that depends on SWBAND and KBL.
*
      ROWSKIP = MIN0( 5, KBL )
*[TP] ROWSKIP is a tuning parameter.
*
      LKAHEAD = 1
*[TP] LKAHEAD is a tuning parameter.
*
*     Quasi block transformations, using the lower (upper) triangular
*     structure of the input matrix. The quasi-block-cycling usually
*     invokes cubic convergence. Big part of this cycle is done inside
*     canonical subspaces of dimensions less than M.
*
      IF( ( LOWER .OR. UPPER ) .AND. ( N.GT.MAX0( 64, 4*KBL ) ) ) THEN
*[TP] The number of partition levels and the actual partition are
*     tuning parameters.
         N4 = N / 4
         N2 = N / 2
         N34 = 3*N4
         IF( APPLV ) THEN
            q = 0
         ELSE
            q = 1
         END IF
*
         IF( LOWER ) THEN
*
*     This works very well on lower triangular matrices, in particular
*     in the framework of the preconditioned Jacobi SVD (xGEJSV).
*     The idea is simple:
*     [+ 0 0 0]   Note that Jacobi transformations of [0 0]
*     [+ + 0 0]                                       [0 0]
*     [+ + x 0]   actually work on [x 0]              [x 0]
*     [+ + x x]                    [x x].             [x x]
*
            CALL DGSVJ0( JOBV, M-N34, N-N34, A( N34+1, N34+1 ), LDA,
     +                   WORK( N34+1 ), SVA( N34+1 ), MVL,
     +                   V( N34*q+1, N34+1 ), LDV, EPSILON, SFMIN, TOL,
     +                   2, WORK( N+1 ), LWORK-N, IERR )
*
            CALL DGSVJ0( JOBV, M-N2, N34-N2, A( N2+1, N2+1 ), LDA,
     +                   WORK( N2+1 ), SVA( N2+1 ), MVL,
     +                   V( N2*q+1, N2+1 ), LDV, EPSILON, SFMIN, TOL, 2,
     +                   WORK( N+1 ), LWORK-N, IERR )
*
            CALL DGSVJ1( JOBV, M-N2, N-N2, N4, A( N2+1, N2+1 ), LDA,
     +                   WORK( N2+1 ), SVA( N2+1 ), MVL,
     +                   V( N2*q+1, N2+1 ), LDV, EPSILON, SFMIN, TOL, 1,
     +                   WORK( N+1 ), LWORK-N, IERR )
*
            CALL DGSVJ0( JOBV, M-N4, N2-N4, A( N4+1, N4+1 ), LDA,
     +                   WORK( N4+1 ), SVA( N4+1 ), MVL,
     +                   V( N4*q+1, N4+1 ), LDV, EPSILON, SFMIN, TOL, 1,
     +                   WORK( N+1 ), LWORK-N, IERR )
*
            CALL DGSVJ0( JOBV, M, N4, A, LDA, WORK, SVA, MVL, V, LDV,
     +                   EPSILON, SFMIN, TOL, 1, WORK( N+1 ), LWORK-N,
     +                   IERR )
*
            CALL DGSVJ1( JOBV, M, N2, N4, A, LDA, WORK, SVA, MVL, V,
     +                   LDV, EPSILON, SFMIN, TOL, 1, WORK( N+1 ),
     +                   LWORK-N, IERR )
*
*
         ELSE IF( UPPER ) THEN
*
*
            CALL DGSVJ0( JOBV, N4, N4, A, LDA, WORK, SVA, MVL, V, LDV,
     +                   EPSILON, SFMIN, TOL, 2, WORK( N+1 ), LWORK-N,
     +                   IERR )
*
            CALL DGSVJ0( JOBV, N2, N4, A( 1, N4+1 ), LDA, WORK( N4+1 ),
     +                   SVA( N4+1 ), MVL, V( N4*q+1, N4+1 ), LDV,
     +                   EPSILON, SFMIN, TOL, 1, WORK( N+1 ), LWORK-N,
     +                   IERR )
*
            CALL DGSVJ1( JOBV, N2, N2, N4, A, LDA, WORK, SVA, MVL, V,
     +                   LDV, EPSILON, SFMIN, TOL, 1, WORK( N+1 ),
     +                   LWORK-N, IERR )
*
            CALL DGSVJ0( JOBV, N2+N4, N4, A( 1, N2+1 ), LDA,
     +                   WORK( N2+1 ), SVA( N2+1 ), MVL,
     +                   V( N2*q+1, N2+1 ), LDV, EPSILON, SFMIN, TOL, 1,
     +                   WORK( N+1 ), LWORK-N, IERR )

         END IF
*
      END IF
*
*     .. Row-cyclic pivot strategy with de Rijk's pivoting ..
*
      DO 1993 i = 1, NSWEEP
*
*     .. go go go ...
*
         MXAAPQ = ZERO
         MXSINJ = ZERO
         ISWROT = 0
*
         NOTROT = 0
         PSKIPPED = 0
*
*     Each sweep is unrolled using KBL-by-KBL tiles over the pivot pairs
*     1 <= p < q <= N. This is the first step toward a blocked implementation
*     of the rotations. New implementation, based on block transformations,
*     is under development.
*
         DO 2000 ibr = 1, NBL
*
            igl = ( ibr-1 )*KBL + 1
*
            DO 1002 ir1 = 0, MIN0( LKAHEAD, NBL-ibr )
*
               igl = igl + ir1*KBL
*
               DO 2001 p = igl, MIN0( igl+KBL-1, N-1 )
*
*     .. de Rijk's pivoting
*
                  q = IDAMAX( N-p+1, SVA( p ), 1 ) + p - 1
                  IF( p.NE.q ) THEN
                     CALL DSWAP( M, A( 1, p ), 1, A( 1, q ), 1 )
                     IF( RSVEC )CALL DSWAP( MVL, V( 1, p ), 1,
     +                                      V( 1, q ), 1 )
                     TEMP1 = SVA( p )
                     SVA( p ) = SVA( q )
                     SVA( q ) = TEMP1
                     TEMP1 = WORK( p )
                     WORK( p ) = WORK( q )
                     WORK( q ) = TEMP1
                  END IF
*
                  IF( ir1.EQ.0 ) THEN
*
*        Column norms are periodically updated by explicit
*        norm computation.
*        Caveat:
*        Unfortunately, some BLAS implementations compute DNRM2(M,A(1,p),1)
*        as DSQRT(DDOT(M,A(1,p),1,A(1,p),1)), which may cause the result to
*        overflow for ||A(:,p)||_2 > DSQRT(overflow_threshold), and to
*        underflow for ||A(:,p)||_2 < DSQRT(underflow_threshold).
*        Hence, DNRM2 cannot be trusted, not even in the case when
*        the true norm is far from the under(over)flow boundaries.
*        If properly implemented DNRM2 is available, the IF-THEN-ELSE
*        below should read "AAPP = DNRM2( M, A(1,p), 1 ) * WORK(p)".
*
                     IF( ( SVA( p ).LT.ROOTBIG ) .AND.
     +                   ( SVA( p ).GT.ROOTSFMIN ) ) THEN
                        SVA( p ) = DNRM2( M, A( 1, p ), 1 )*WORK( p )
                     ELSE
                        TEMP1 = ZERO
                        AAPP = ZERO
                        CALL DLASSQ( M, A( 1, p ), 1, TEMP1, AAPP )
                        SVA( p ) = TEMP1*DSQRT( AAPP )*WORK( p )
                     END IF
                     AAPP = SVA( p )
                  ELSE
                     AAPP = SVA( p )
                  END IF
*
                  IF( AAPP.GT.ZERO ) THEN
*
                     PSKIPPED = 0
*
                     DO 2002 q = p + 1, MIN0( igl+KBL-1, N )
*
                        AAQQ = SVA( q )
*
                        IF( AAQQ.GT.ZERO ) THEN
*
                           AAPP0 = AAPP
                           IF( AAQQ.GE.ONE ) THEN
                              ROTOK = ( SMALL*AAPP ).LE.AAQQ
                              IF( AAPP.LT.( BIG / AAQQ ) ) THEN
                                 AAPQ = ( DDOT( M, A( 1, p ), 1, A( 1,
     +                                  q ), 1 )*WORK( p )*WORK( q ) /
     +                                  AAQQ ) / AAPP
                              ELSE
                                 CALL DCOPY( M, A( 1, p ), 1,
     +                                       WORK( N+1 ), 1 )
                                 CALL DLASCL( 'G', 0, 0, AAPP,
     +                                        WORK( p ), M, 1,
     +                                        WORK( N+1 ), LDA, IERR )
                                 AAPQ = DDOT( M, WORK( N+1 ), 1,
     +                                  A( 1, q ), 1 )*WORK( q ) / AAQQ
                              END IF
                           ELSE
                              ROTOK = AAPP.LE.( AAQQ / SMALL )
                              IF( AAPP.GT.( SMALL / AAQQ ) ) THEN
                                 AAPQ = ( DDOT( M, A( 1, p ), 1, A( 1,
     +                                  q ), 1 )*WORK( p )*WORK( q ) /
     +                                  AAQQ ) / AAPP
                              ELSE
                                 CALL DCOPY( M, A( 1, q ), 1,
     +                                       WORK( N+1 ), 1 )
                                 CALL DLASCL( 'G', 0, 0, AAQQ,
     +                                        WORK( q ), M, 1,
     +                                        WORK( N+1 ), LDA, IERR )
                                 AAPQ = DDOT( M, WORK( N+1 ), 1,
     +                                  A( 1, p ), 1 )*WORK( p ) / AAPP
                              END IF
                           END IF
*
                           MXAAPQ = DMAX1( MXAAPQ, DABS( AAPQ ) )
*
*        TO rotate or NOT to rotate, THAT is the question ...
*
                           IF( DABS( AAPQ ).GT.TOL ) THEN
*
*           .. rotate
*[RTD]      ROTATED = ROTATED + ONE
*
                              IF( ir1.EQ.0 ) THEN
                                 NOTROT = 0
                                 PSKIPPED = 0
                                 ISWROT = ISWROT + 1
                              END IF
*
                              IF( ROTOK ) THEN
*
                                 AQOAP = AAQQ / AAPP
                                 APOAQ = AAPP / AAQQ
                                 THETA = -HALF*DABS( AQOAP-APOAQ ) /
     +                                   AAPQ
*
                                 IF( DABS( THETA ).GT.BIGTHETA ) THEN
*
                                    T = HALF / THETA
                                    FASTR( 3 ) = T*WORK( p ) / WORK( q )
                                    FASTR( 4 ) = -T*WORK( q ) /
     +                                           WORK( p )
                                    CALL DROTM( M, A( 1, p ), 1,
     +                                          A( 1, q ), 1, FASTR )
                                    IF( RSVEC )CALL DROTM( MVL,
     +                                              V( 1, p ), 1,
     +                                              V( 1, q ), 1,
     +                                              FASTR )
                                    SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                         ONE+T*APOAQ*AAPQ ) )
                                    AAPP = AAPP*DSQRT( ONE-T*AQOAP*
     +                                     AAPQ )
                                    MXSINJ = DMAX1( MXSINJ, DABS( T ) )
*
                                 ELSE
*
*                 .. choose correct signum for THETA and rotate
*
                                    THSIGN = -DSIGN( ONE, AAPQ )
                                    T = ONE / ( THETA+THSIGN*
     +                                  DSQRT( ONE+THETA*THETA ) )
                                    CS = DSQRT( ONE / ( ONE+T*T ) )
                                    SN = T*CS
*
                                    MXSINJ = DMAX1( MXSINJ, DABS( SN ) )
                                    SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                         ONE+T*APOAQ*AAPQ ) )
                                    AAPP = AAPP*DSQRT( DMAX1( ZERO,
     +                                     ONE-T*AQOAP*AAPQ ) )
*
                                    APOAQ = WORK( p ) / WORK( q )
                                    AQOAP = WORK( q ) / WORK( p )
                                    IF( WORK( p ).GE.ONE ) THEN
                                       IF( WORK( q ).GE.ONE ) THEN
                                          FASTR( 3 ) = T*APOAQ
                                          FASTR( 4 ) = -T*AQOAP
                                          WORK( p ) = WORK( p )*CS
                                          WORK( q ) = WORK( q )*CS
                                          CALL DROTM( M, A( 1, p ), 1,
     +                                                A( 1, q ), 1,
     +                                                FASTR )
                                          IF( RSVEC )CALL DROTM( MVL,
     +                                        V( 1, p ), 1, V( 1, q ),
     +                                        1, FASTR )
                                       ELSE
                                          CALL DAXPY( M, -T*AQOAP,
     +                                                A( 1, q ), 1,
     +                                                A( 1, p ), 1 )
                                          CALL DAXPY( M, CS*SN*APOAQ,
     +                                                A( 1, p ), 1,
     +                                                A( 1, q ), 1 )
                                          WORK( p ) = WORK( p )*CS
                                          WORK( q ) = WORK( q ) / CS
                                          IF( RSVEC ) THEN
                                             CALL DAXPY( MVL, -T*AQOAP,
     +                                                   V( 1, q ), 1,
     +                                                   V( 1, p ), 1 )
                                             CALL DAXPY( MVL,
     +                                                   CS*SN*APOAQ,
     +                                                   V( 1, p ), 1,
     +                                                   V( 1, q ), 1 )
                                          END IF
                                       END IF
                                    ELSE
                                       IF( WORK( q ).GE.ONE ) THEN
                                          CALL DAXPY( M, T*APOAQ,
     +                                                A( 1, p ), 1,
     +                                                A( 1, q ), 1 )
                                          CALL DAXPY( M, -CS*SN*AQOAP,
     +                                                A( 1, q ), 1,
     +                                                A( 1, p ), 1 )
                                          WORK( p ) = WORK( p ) / CS
                                          WORK( q ) = WORK( q )*CS
                                          IF( RSVEC ) THEN
                                             CALL DAXPY( MVL, T*APOAQ,
     +                                                   V( 1, p ), 1,
     +                                                   V( 1, q ), 1 )
                                             CALL DAXPY( MVL,
     +                                                   -CS*SN*AQOAP,
     +                                                   V( 1, q ), 1,
     +                                                   V( 1, p ), 1 )
                                          END IF
                                       ELSE
                                          IF( WORK( p ).GE.WORK( q ) )
     +                                        THEN
                                             CALL DAXPY( M, -T*AQOAP,
     +                                                   A( 1, q ), 1,
     +                                                   A( 1, p ), 1 )
                                             CALL DAXPY( M, CS*SN*APOAQ,
     +                                                   A( 1, p ), 1,
     +                                                   A( 1, q ), 1 )
                                             WORK( p ) = WORK( p )*CS
                                             WORK( q ) = WORK( q ) / CS
                                             IF( RSVEC ) THEN
                                                CALL DAXPY( MVL,
     +                                               -T*AQOAP,
     +                                               V( 1, q ), 1,
     +                                               V( 1, p ), 1 )
                                                CALL DAXPY( MVL,
     +                                               CS*SN*APOAQ,
     +                                               V( 1, p ), 1,
     +                                               V( 1, q ), 1 )
                                             END IF
                                          ELSE
                                             CALL DAXPY( M, T*APOAQ,
     +                                                   A( 1, p ), 1,
     +                                                   A( 1, q ), 1 )
                                             CALL DAXPY( M,
     +                                                   -CS*SN*AQOAP,
     +                                                   A( 1, q ), 1,
     +                                                   A( 1, p ), 1 )
                                             WORK( p ) = WORK( p ) / CS
                                             WORK( q ) = WORK( q )*CS
                                             IF( RSVEC ) THEN
                                                CALL DAXPY( MVL,
     +                                               T*APOAQ, V( 1, p ),
     +                                               1, V( 1, q ), 1 )
                                                CALL DAXPY( MVL,
     +                                               -CS*SN*AQOAP,
     +                                               V( 1, q ), 1,
     +                                               V( 1, p ), 1 )
                                             END IF
                                          END IF
                                       END IF
                                    END IF
                                 END IF
*
                              ELSE
*              .. have to use modified Gram-Schmidt like transformation
                                 CALL DCOPY( M, A( 1, p ), 1,
     +                                       WORK( N+1 ), 1 )
                                 CALL DLASCL( 'G', 0, 0, AAPP, ONE, M,
     +                                        1, WORK( N+1 ), LDA,
     +                                        IERR )
                                 CALL DLASCL( 'G', 0, 0, AAQQ, ONE, M,
     +                                        1, A( 1, q ), LDA, IERR )
                                 TEMP1 = -AAPQ*WORK( p ) / WORK( q )
                                 CALL DAXPY( M, TEMP1, WORK( N+1 ), 1,
     +                                       A( 1, q ), 1 )
                                 CALL DLASCL( 'G', 0, 0, ONE, AAQQ, M,
     +                                        1, A( 1, q ), LDA, IERR )
                                 SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                      ONE-AAPQ*AAPQ ) )
                                 MXSINJ = DMAX1( MXSINJ, SFMIN )
                              END IF
*           END IF ROTOK THEN ... ELSE
*
*           In the case of cancellation in updating SVA(q), SVA(p)
*           recompute SVA(q), SVA(p).
*
                              IF( ( SVA( q ) / AAQQ )**2.LE.ROOTEPS )
     +                            THEN
                                 IF( ( AAQQ.LT.ROOTBIG ) .AND.
     +                               ( AAQQ.GT.ROOTSFMIN ) ) THEN
                                    SVA( q ) = DNRM2( M, A( 1, q ), 1 )*
     +                                         WORK( q )
                                 ELSE
                                    T = ZERO
                                    AAQQ = ZERO
                                    CALL DLASSQ( M, A( 1, q ), 1, T,
     +                                           AAQQ )
                                    SVA( q ) = T*DSQRT( AAQQ )*WORK( q )
                                 END IF
                              END IF
                              IF( ( AAPP / AAPP0 ).LE.ROOTEPS ) THEN
                                 IF( ( AAPP.LT.ROOTBIG ) .AND.
     +                               ( AAPP.GT.ROOTSFMIN ) ) THEN
                                    AAPP = DNRM2( M, A( 1, p ), 1 )*
     +                                     WORK( p )
                                 ELSE
                                    T = ZERO
                                    AAPP = ZERO
                                    CALL DLASSQ( M, A( 1, p ), 1, T,
     +                                           AAPP )
                                    AAPP = T*DSQRT( AAPP )*WORK( p )
                                 END IF
                                 SVA( p ) = AAPP
                              END IF
*
                           ELSE
*        A(:,p) and A(:,q) already numerically orthogonal
                              IF( ir1.EQ.0 )NOTROT = NOTROT + 1
*[RTD]      SKIPPED  = SKIPPED  + 1
                              PSKIPPED = PSKIPPED + 1
                           END IF
                        ELSE
*        A(:,q) is zero column
                           IF( ir1.EQ.0 )NOTROT = NOTROT + 1
                           PSKIPPED = PSKIPPED + 1
                        END IF
*
                        IF( ( i.LE.SWBAND ) .AND.
     +                      ( PSKIPPED.GT.ROWSKIP ) ) THEN
                           IF( ir1.EQ.0 )AAPP = -AAPP
                           NOTROT = 0
                           GO TO 2103
                        END IF
*
 2002                CONTINUE
*     END q-LOOP
*
 2103                CONTINUE
*     bailed out of q-loop
*
                     SVA( p ) = AAPP
*
                  ELSE
                     SVA( p ) = AAPP
                     IF( ( ir1.EQ.0 ) .AND. ( AAPP.EQ.ZERO ) )
     +                   NOTROT = NOTROT + MIN0( igl+KBL-1, N ) - p
                  END IF
*
 2001          CONTINUE
*     end of the p-loop
*     end of doing the block ( ibr, ibr )
 1002       CONTINUE
*     end of ir1-loop
*
* ... go to the off diagonal blocks
*
            igl = ( ibr-1 )*KBL + 1
*
            DO 2010 jbc = ibr + 1, NBL
*
               jgl = ( jbc-1 )*KBL + 1
*
*        doing the block at ( ibr, jbc )
*
               IJBLSK = 0
               DO 2100 p = igl, MIN0( igl+KBL-1, N )
*
                  AAPP = SVA( p )
                  IF( AAPP.GT.ZERO ) THEN
*
                     PSKIPPED = 0
*
                     DO 2200 q = jgl, MIN0( jgl+KBL-1, N )
*
                        AAQQ = SVA( q )
                        IF( AAQQ.GT.ZERO ) THEN
                           AAPP0 = AAPP
*
*     .. M x 2 Jacobi SVD ..
*
*        Safe Gram matrix computation
*
                           IF( AAQQ.GE.ONE ) THEN
                              IF( AAPP.GE.AAQQ ) THEN
                                 ROTOK = ( SMALL*AAPP ).LE.AAQQ
                              ELSE
                                 ROTOK = ( SMALL*AAQQ ).LE.AAPP
                              END IF
                              IF( AAPP.LT.( BIG / AAQQ ) ) THEN
                                 AAPQ = ( DDOT( M, A( 1, p ), 1, A( 1,
     +                                  q ), 1 )*WORK( p )*WORK( q ) /
     +                                  AAQQ ) / AAPP
                              ELSE
                                 CALL DCOPY( M, A( 1, p ), 1,
     +                                       WORK( N+1 ), 1 )
                                 CALL DLASCL( 'G', 0, 0, AAPP,
     +                                        WORK( p ), M, 1,
     +                                        WORK( N+1 ), LDA, IERR )
                                 AAPQ = DDOT( M, WORK( N+1 ), 1,
     +                                  A( 1, q ), 1 )*WORK( q ) / AAQQ
                              END IF
                           ELSE
                              IF( AAPP.GE.AAQQ ) THEN
                                 ROTOK = AAPP.LE.( AAQQ / SMALL )
                              ELSE
                                 ROTOK = AAQQ.LE.( AAPP / SMALL )
                              END IF
                              IF( AAPP.GT.( SMALL / AAQQ ) ) THEN
                                 AAPQ = ( DDOT( M, A( 1, p ), 1, A( 1,
     +                                  q ), 1 )*WORK( p )*WORK( q ) /
     +                                  AAQQ ) / AAPP
                              ELSE
                                 CALL DCOPY( M, A( 1, q ), 1,
     +                                       WORK( N+1 ), 1 )
                                 CALL DLASCL( 'G', 0, 0, AAQQ,
     +                                        WORK( q ), M, 1,
     +                                        WORK( N+1 ), LDA, IERR )
                                 AAPQ = DDOT( M, WORK( N+1 ), 1,
     +                                  A( 1, p ), 1 )*WORK( p ) / AAPP
                              END IF
                           END IF
*
                           MXAAPQ = DMAX1( MXAAPQ, DABS( AAPQ ) )
*
*        TO rotate or NOT to rotate, THAT is the question ...
*
                           IF( DABS( AAPQ ).GT.TOL ) THEN
                              NOTROT = 0
*[RTD]      ROTATED  = ROTATED + 1
                              PSKIPPED = 0
                              ISWROT = ISWROT + 1
*
                              IF( ROTOK ) THEN
*
                                 AQOAP = AAQQ / AAPP
                                 APOAQ = AAPP / AAQQ
                                 THETA = -HALF*DABS( AQOAP-APOAQ ) /
     +                                   AAPQ
                                 IF( AAQQ.GT.AAPP0 )THETA = -THETA
*
                                 IF( DABS( THETA ).GT.BIGTHETA ) THEN
                                    T = HALF / THETA
                                    FASTR( 3 ) = T*WORK( p ) / WORK( q )
                                    FASTR( 4 ) = -T*WORK( q ) /
     +                                           WORK( p )
                                    CALL DROTM( M, A( 1, p ), 1,
     +                                          A( 1, q ), 1, FASTR )
                                    IF( RSVEC )CALL DROTM( MVL,
     +                                              V( 1, p ), 1,
     +                                              V( 1, q ), 1,
     +                                              FASTR )
                                    SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                         ONE+T*APOAQ*AAPQ ) )
                                    AAPP = AAPP*DSQRT( DMAX1( ZERO,
     +                                     ONE-T*AQOAP*AAPQ ) )
                                    MXSINJ = DMAX1( MXSINJ, DABS( T ) )
                                 ELSE
*
*                 .. choose correct signum for THETA and rotate
*
                                    THSIGN = -DSIGN( ONE, AAPQ )
                                    IF( AAQQ.GT.AAPP0 )THSIGN = -THSIGN
                                    T = ONE / ( THETA+THSIGN*
     +                                  DSQRT( ONE+THETA*THETA ) )
                                    CS = DSQRT( ONE / ( ONE+T*T ) )
                                    SN = T*CS
                                    MXSINJ = DMAX1( MXSINJ, DABS( SN ) )
                                    SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                         ONE+T*APOAQ*AAPQ ) )
                                    AAPP = AAPP*DSQRT( ONE-T*AQOAP*
     +                                     AAPQ )
*
                                    APOAQ = WORK( p ) / WORK( q )
                                    AQOAP = WORK( q ) / WORK( p )
                                    IF( WORK( p ).GE.ONE ) THEN
*
                                       IF( WORK( q ).GE.ONE ) THEN
                                          FASTR( 3 ) = T*APOAQ
                                          FASTR( 4 ) = -T*AQOAP
                                          WORK( p ) = WORK( p )*CS
                                          WORK( q ) = WORK( q )*CS
                                          CALL DROTM( M, A( 1, p ), 1,
     +                                                A( 1, q ), 1,
     +                                                FASTR )
                                          IF( RSVEC )CALL DROTM( MVL,
     +                                        V( 1, p ), 1, V( 1, q ),
     +                                        1, FASTR )
                                       ELSE
                                          CALL DAXPY( M, -T*AQOAP,
     +                                                A( 1, q ), 1,
     +                                                A( 1, p ), 1 )
                                          CALL DAXPY( M, CS*SN*APOAQ,
     +                                                A( 1, p ), 1,
     +                                                A( 1, q ), 1 )
                                          IF( RSVEC ) THEN
                                             CALL DAXPY( MVL, -T*AQOAP,
     +                                                   V( 1, q ), 1,
     +                                                   V( 1, p ), 1 )
                                             CALL DAXPY( MVL,
     +                                                   CS*SN*APOAQ,
     +                                                   V( 1, p ), 1,
     +                                                   V( 1, q ), 1 )
                                          END IF
                                          WORK( p ) = WORK( p )*CS
                                          WORK( q ) = WORK( q ) / CS
                                       END IF
                                    ELSE
                                       IF( WORK( q ).GE.ONE ) THEN
                                          CALL DAXPY( M, T*APOAQ,
     +                                                A( 1, p ), 1,
     +                                                A( 1, q ), 1 )
                                          CALL DAXPY( M, -CS*SN*AQOAP,
     +                                                A( 1, q ), 1,
     +                                                A( 1, p ), 1 )
                                          IF( RSVEC ) THEN
                                             CALL DAXPY( MVL, T*APOAQ,
     +                                                   V( 1, p ), 1,
     +                                                   V( 1, q ), 1 )
                                             CALL DAXPY( MVL,
     +                                                   -CS*SN*AQOAP,
     +                                                   V( 1, q ), 1,
     +                                                   V( 1, p ), 1 )
                                          END IF
                                          WORK( p ) = WORK( p ) / CS
                                          WORK( q ) = WORK( q )*CS
                                       ELSE
                                          IF( WORK( p ).GE.WORK( q ) )
     +                                        THEN
                                             CALL DAXPY( M, -T*AQOAP,
     +                                                   A( 1, q ), 1,
     +                                                   A( 1, p ), 1 )
                                             CALL DAXPY( M, CS*SN*APOAQ,
     +                                                   A( 1, p ), 1,
     +                                                   A( 1, q ), 1 )
                                             WORK( p ) = WORK( p )*CS
                                             WORK( q ) = WORK( q ) / CS
                                             IF( RSVEC ) THEN
                                                CALL DAXPY( MVL,
     +                                               -T*AQOAP,
     +                                               V( 1, q ), 1,
     +                                               V( 1, p ), 1 )
                                                CALL DAXPY( MVL,
     +                                               CS*SN*APOAQ,
     +                                               V( 1, p ), 1,
     +                                               V( 1, q ), 1 )
                                             END IF
                                          ELSE
                                             CALL DAXPY( M, T*APOAQ,
     +                                                   A( 1, p ), 1,
     +                                                   A( 1, q ), 1 )
                                             CALL DAXPY( M,
     +                                                   -CS*SN*AQOAP,
     +                                                   A( 1, q ), 1,
     +                                                   A( 1, p ), 1 )
                                             WORK( p ) = WORK( p ) / CS
                                             WORK( q ) = WORK( q )*CS
                                             IF( RSVEC ) THEN
                                                CALL DAXPY( MVL,
     +                                               T*APOAQ, V( 1, p ),
     +                                               1, V( 1, q ), 1 )
                                                CALL DAXPY( MVL,
     +                                               -CS*SN*AQOAP,
     +                                               V( 1, q ), 1,
     +                                               V( 1, p ), 1 )
                                             END IF
                                          END IF
                                       END IF
                                    END IF
                                 END IF
*
                              ELSE
                                 IF( AAPP.GT.AAQQ ) THEN
                                    CALL DCOPY( M, A( 1, p ), 1,
     +                                          WORK( N+1 ), 1 )
                                    CALL DLASCL( 'G', 0, 0, AAPP, ONE,
     +                                           M, 1, WORK( N+1 ), LDA,
     +                                           IERR )
                                    CALL DLASCL( 'G', 0, 0, AAQQ, ONE,
     +                                           M, 1, A( 1, q ), LDA,
     +                                           IERR )
                                    TEMP1 = -AAPQ*WORK( p ) / WORK( q )
                                    CALL DAXPY( M, TEMP1, WORK( N+1 ),
     +                                          1, A( 1, q ), 1 )
                                    CALL DLASCL( 'G', 0, 0, ONE, AAQQ,
     +                                           M, 1, A( 1, q ), LDA,
     +                                           IERR )
                                    SVA( q ) = AAQQ*DSQRT( DMAX1( ZERO,
     +                                         ONE-AAPQ*AAPQ ) )
                                    MXSINJ = DMAX1( MXSINJ, SFMIN )
                                 ELSE
                                    CALL DCOPY( M, A( 1, q ), 1,
     +                                          WORK( N+1 ), 1 )
                                    CALL DLASCL( 'G', 0, 0, AAQQ, ONE,
     +                                           M, 1, WORK( N+1 ), LDA,
     +                                           IERR )
                                    CALL DLASCL( 'G', 0, 0, AAPP, ONE,
     +                                           M, 1, A( 1, p ), LDA,
     +                                           IERR )
                                    TEMP1 = -AAPQ*WORK( q ) / WORK( p )
                                    CALL DAXPY( M, TEMP1, WORK( N+1 ),
     +                                          1, A( 1, p ), 1 )
                                    CALL DLASCL( 'G', 0, 0, ONE, AAPP,
     +                                           M, 1, A( 1, p ), LDA,
     +                                           IERR )
                                    SVA( p ) = AAPP*DSQRT( DMAX1( ZERO,
     +                                         ONE-AAPQ*AAPQ ) )
                                    MXSINJ = DMAX1( MXSINJ, SFMIN )
                                 END IF
                              END IF
*           END IF ROTOK THEN ... ELSE
*
*           In the case of cancellation in updating SVA(q)
*           .. recompute SVA(q)
                              IF( ( SVA( q ) / AAQQ )**2.LE.ROOTEPS )
     +                            THEN
                                 IF( ( AAQQ.LT.ROOTBIG ) .AND.
     +                               ( AAQQ.GT.ROOTSFMIN ) ) THEN
                                    SVA( q ) = DNRM2( M, A( 1, q ), 1 )*
     +                                         WORK( q )
                                 ELSE
                                    T = ZERO
                                    AAQQ = ZERO
                                    CALL DLASSQ( M, A( 1, q ), 1, T,
     +                                           AAQQ )
                                    SVA( q ) = T*DSQRT( AAQQ )*WORK( q )
                                 END IF
                              END IF
                              IF( ( AAPP / AAPP0 )**2.LE.ROOTEPS ) THEN
                                 IF( ( AAPP.LT.ROOTBIG ) .AND.
     +                               ( AAPP.GT.ROOTSFMIN ) ) THEN
                                    AAPP = DNRM2( M, A( 1, p ), 1 )*
     +                                     WORK( p )
                                 ELSE
                                    T = ZERO
                                    AAPP = ZERO
                                    CALL DLASSQ( M, A( 1, p ), 1, T,
     +                                           AAPP )
                                    AAPP = T*DSQRT( AAPP )*WORK( p )
                                 END IF
                                 SVA( p ) = AAPP
                              END IF
*              end of OK rotation
                           ELSE
                              NOTROT = NOTROT + 1
*[RTD]      SKIPPED  = SKIPPED  + 1
                              PSKIPPED = PSKIPPED + 1
                              IJBLSK = IJBLSK + 1
                           END IF
                        ELSE
                           NOTROT = NOTROT + 1
                           PSKIPPED = PSKIPPED + 1
                           IJBLSK = IJBLSK + 1
                        END IF
*
                        IF( ( i.LE.SWBAND ) .AND. ( IJBLSK.GE.BLSKIP ) )
     +                      THEN
                           SVA( p ) = AAPP
                           NOTROT = 0
                           GO TO 2011
                        END IF
                        IF( ( i.LE.SWBAND ) .AND.
     +                      ( PSKIPPED.GT.ROWSKIP ) ) THEN
                           AAPP = -AAPP
                           NOTROT = 0
                           GO TO 2203
                        END IF
*
 2200                CONTINUE
*        end of the q-loop
 2203                CONTINUE
*
                     SVA( p ) = AAPP
*
                  ELSE
*
                     IF( AAPP.EQ.ZERO )NOTROT = NOTROT +
     +                   MIN0( jgl+KBL-1, N ) - jgl + 1
                     IF( AAPP.LT.ZERO )NOTROT = 0
*
                  END IF
*
 2100          CONTINUE
*     end of the p-loop
 2010       CONTINUE
*     end of the jbc-loop
 2011       CONTINUE
*2011 bailed out of the jbc-loop
            DO 2012 p = igl, MIN0( igl+KBL-1, N )
               SVA( p ) = DABS( SVA( p ) )
 2012       CONTINUE
***
 2000    CONTINUE
*2000 :: end of the ibr-loop
*
*     .. update SVA(N)
         IF( ( SVA( N ).LT.ROOTBIG ) .AND. ( SVA( N ).GT.ROOTSFMIN ) )
     +       THEN
            SVA( N ) = DNRM2( M, A( 1, N ), 1 )*WORK( N )
         ELSE
            T = ZERO
            AAPP = ZERO
            CALL DLASSQ( M, A( 1, N ), 1, T, AAPP )
            SVA( N ) = T*DSQRT( AAPP )*WORK( N )
         END IF
*
*     Additional steering devices
*
         IF( ( i.LT.SWBAND ) .AND. ( ( MXAAPQ.LE.ROOTTOL ) .OR.
     +       ( ISWROT.LE.N ) ) )SWBAND = i
*
         IF( ( i.GT.SWBAND+1 ) .AND. ( MXAAPQ.LT.DSQRT( DBLE( N ) )*
     +       TOL ) .AND. ( DBLE( N )*MXAAPQ*MXSINJ.LT.TOL ) ) THEN
            GO TO 1994
         END IF
*
         IF( NOTROT.GE.EMPTSW )GO TO 1994
*
 1993 CONTINUE
*     end i=1:NSWEEP loop
*
* #:( Reaching this point means that the procedure has not converged.
      INFO = NSWEEP - 1
      GO TO 1995
*
 1994 CONTINUE
* #:) Reaching this point means numerical convergence after the i-th
*     sweep.
*
      INFO = 0
* #:) INFO = 0 confirms successful iterations.
 1995 CONTINUE
*
*     Sort the singular values and find how many are above
*     the underflow threshold.
*
      N2 = 0
      N4 = 0
      DO 5991 p = 1, N - 1
         q = IDAMAX( N-p+1, SVA( p ), 1 ) + p - 1
         IF( p.NE.q ) THEN
            TEMP1 = SVA( p )
            SVA( p ) = SVA( q )
            SVA( q ) = TEMP1
            TEMP1 = WORK( p )
            WORK( p ) = WORK( q )
            WORK( q ) = TEMP1
            CALL DSWAP( M, A( 1, p ), 1, A( 1, q ), 1 )
            IF( RSVEC )CALL DSWAP( MVL, V( 1, p ), 1, V( 1, q ), 1 )
         END IF
         IF( SVA( p ).NE.ZERO ) THEN
            N4 = N4 + 1
            IF( SVA( p )*SCALE.GT.SFMIN )N2 = N2 + 1
         END IF
 5991 CONTINUE
      IF( SVA( N ).NE.ZERO ) THEN
         N4 = N4 + 1
         IF( SVA( N )*SCALE.GT.SFMIN )N2 = N2 + 1
      END IF
*
*     Normalize the left singular vectors.
*
      IF( LSVEC .OR. UCTOL ) THEN
         DO 1998 p = 1, N2
            CALL DSCAL( M, WORK( p ) / SVA( p ), A( 1, p ), 1 )
 1998    CONTINUE
      END IF
*
*     Scale the product of Jacobi rotations (assemble the fast rotations).
*
      IF( RSVEC ) THEN
         IF( APPLV ) THEN
            DO 2398 p = 1, N
               CALL DSCAL( MVL, WORK( p ), V( 1, p ), 1 )
 2398       CONTINUE
         ELSE
            DO 2399 p = 1, N
               TEMP1 = ONE / DNRM2( MVL, V( 1, p ), 1 )
               CALL DSCAL( MVL, TEMP1, V( 1, p ), 1 )
 2399       CONTINUE
         END IF
      END IF
*
*     Undo scaling, if necessary (and possible).
      IF( ( ( SCALE.GT.ONE ) .AND. ( SVA( 1 ).LT.( BIG /
     +    SCALE ) ) ) .OR. ( ( SCALE.LT.ONE ) .AND. ( SVA( N2 ).GT.
     +    ( SFMIN / SCALE ) ) ) ) THEN
         DO 2400 p = 1, N
            SVA( p ) = SCALE*SVA( p )
 2400    CONTINUE
         SCALE = ONE
      END IF
*
      WORK( 1 ) = SCALE
*     The singular values of A are SCALE*SVA(1:N). If SCALE.NE.ONE
*     then some of the singular values may overflow or underflow and
*     the spectrum is given in this factored representation.
*
      WORK( 2 ) = DBLE( N4 )
*     N4 is the number of computed nonzero singular values of A.
*
      WORK( 3 ) = DBLE( N2 )
*     N2 is the number of singular values of A greater than SFMIN.
*     If N2<N, SVA(N2:N) contains ZEROS and/or denormalized numbers
*     that may carry some information.
*
      WORK( 4 ) = DBLE( i )
*     i is the index of the last sweep before declaring convergence.
*
      WORK( 5 ) = MXAAPQ
*     MXAAPQ is the largest absolute value of scaled pivots in the
*     last sweep
*
      WORK( 6 ) = MXSINJ
*     MXSINJ is the largest absolute value of the sines of Jacobi angles
*     in the last sweep
*
      RETURN
*     ..
*     .. END OF DGESVJ
*     ..
      END
