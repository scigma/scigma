c***********************************************************************
c
c     Date: Sun, 13 Nov 88 09:55:27 cst
c     From: netlibd@antares.mcs.anl.gov (Netlib)
c     Message-Id: <8811131555.AA18963@surfer.mcs.anl.gov>
c     To: dcrane@icecream.Princeton.EDU
c     Subject: send idamax from linpack
c     Status: R
c
Caveat receptor.  (Jack) dongarra@anl-mcs.arpa, (Eric Grosse) research!ehg
Careful! Anything free comes with no guarantee.
c
c     *** from netlib, Sun Nov 13 09:55:20 CST 1988 ***
c
c***********************************************************************






      integer function idamax(n,dx,incx)
c
c     finds the index of element having max. absolute value.
c     jack dongarra, linpack, 3/11/78.
c
      double precision dx(1),dmax
      integer i,incx,ix,n
c
      idamax = 0
      if( n .lt. 1 ) return
      idamax = 1
      if(n.eq.1)return
      if(incx.eq.1)go to 20
c
c        code for increment not equal to 1
c
      ix = 1
      dmax = dabs(dx(1))
      ix = ix + incx
      do 10 i = 2,n
         if(dabs(dx(ix)).le.dmax) go to 5
         idamax = i
         dmax = dabs(dx(ix))
    5    ix = ix + incx
   10 continue
      return
c
c        code for increment equal to 1
c
   20 dmax = dabs(dx(1))
      do 30 i = 2,n
         if(dabs(dx(i)).le.dmax) go to 30
         idamax = i
         dmax = dabs(dx(i))
   30 continue
      return
      end
