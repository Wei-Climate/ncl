C
C       $Id: csex07.f,v 1.1 1998-12-10 00:09:10 fred Exp $
C
      PROGRAM CSEX07
C
C  Do a 2D approximation using a list of output coordinates.  
C
      PARAMETER (IERRF=6, LUNIT=2, IWTYPE=1, IWKID=1)
      PARAMETER (NDIM=2, NDATA=500, NX=29, NY=25,
     +           NO=NX*NY, N1=10, N2=10, NCF=N1*N2, NWRK=NCF*(NCF+3))
      DIMENSION XDATA(NDIM,NDATA),YDATA(NDATA),NODES(NDIM),WORK(NWRK)
      DATA XMIN,YMIN,XMAX,YMAX / -1.4, -1.2, 1.4, 1.2/
      REAL XO(NO),  YO(NO), OUTPUT(NO)
      REAL XP(NX),  YP(NY), OPLOT(NX,NY)
C
C  Create the data array for the surface.
C
      DO 10 I=1,NDATA
        XDATA(1,I) = XMIN+(XMAX-XMIN)*DSRND1()
        XDATA(2,I) = YMIN+(YMAX-YMIN)*DSRND1()
        YDATA(I)   = XDATA(1,I) + XDATA(2,I)
        T1 = 1.0/((ABS(XDATA(1,I)-0.1))**2.75 +
     +                     ABS(XDATA(2,I))**2.75+0.09)
        T2 = 1.0/((ABS(XDATA(1,I)+0.1))**2.75 +
     +                     ABS(XDATA(2,I))**2.75+0.09)
        YDATA(I) = 0.3*(YDATA(I)+T1-T2)
   10 CONTINUE
C
C  Create the output arrays.
C
      INDX = 0
      DO 103 J=1,NY
        DO 104 I=1,NX
          INDX = INDX+1
          XO(INDX) = XMIN+(REAL(I-1)/REAL(NX-1))*(XMAX-XMIN)
          YO(INDX) = YMIN+(REAL(J-1)/REAL(NY-1))*(YMAX-YMIN)
  104   CONTINUE
  103 CONTINUE
C
C  Specify the numbers of nodes in each coordinate.
C
      NODES(1) = N1
      NODES(2) = N2
C
      CALL CSA2LS (NDATA,XDATA,YDATA,NODES,NO,XO,YO,
     +             OUTPUT,NWRK,WORK,IER)
C
C  Plot the 2D surface approximation.
C
C Open GKS and define the foreground and background color.
C
      CALL GOPKS (IERRF, ISZDM)
      CALL GOPWK (IWKID, LUNIT, IWTYPE)
      CALL GACWK (IWKID)
C
C  Convert the linear output array to a 2D array.
C
      DO 21 I=1,NX
        DO 22 J=1,NY
            OPLOT(I,J) = OUTPUT((J-1)*NX + I)
   22   CONTINUE
   21 CONTINUE
C
C  Create the output grid for plotting.
C
      DO 105 I=1,NX
        XP(I) = XMIN+(REAL(I-1)/REAL(NX-1))*(XMAX-XMIN)
  105 CONTINUE
      DO 106 J=1,NY
        YP(J) = YMIN+(REAL(J-1)/REAL(NY-1))*(YMAX-YMIN)
  106 CONTINUE
C
      CALL TDEZ2D(NX, NY, XP, YP, OPLOT, 2.5, -154., 80., 6)
      CALL FRAME()
C
C Close GKS.
C
      CALL GDAWK (IWKID)
      CALL GCLWK (IWKID)
      CALL GCLKS
C
      STOP
      END
      REAL FUNCTION DSRND1()
C
      DATA ISEED/1/
      SAVE ISEED
C
      ISEED = ISEED*1103515245 + 12345
      IT = IAND(ISHIFT(ISEED,-16),32767)
C
      DSRND1 = REAL(IT)/32767.
C
      RETURN
      END
