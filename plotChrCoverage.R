#!/usr/bin/env Rscript
library( gtools);
options(digits=20)
#plotChrCoverage.R [input, produced using bedBamAndBeyond] [output pdf]
args=(commandArgs(TRUE))

data <- read.table(args[1],header=FALSE);
data<- data[mixedorder(data$V1),];

lastposl<-0;
lastposh<-0;
lastposc<-character();

lastposlAdd<-0;
lastposhAdd<-0;

dataN<-data.frame();
dataChr<-character(); #chr name
dataChrl<-numeric();  #lower chr pos
dataChrh<-numeric();  #high  chr pos

for(i in 1:length(data$V1)){
  c<-data[i,1];
  l<-data[i,2]; 
  h<-data[i,3];

  
#  print("l");
#  print(c);
#  print(l);
#  print(h);
  if( l==1 ){ #first record of new chrosome

    if( i!=1){#new chr, not first
      lastposlAdd<-lastposh;
      lastposhAdd<-lastposh;
#      print(c);
#      print(lastposc);

      dataChr<- c(dataChr,  lastposc);  
      dataChrh<-c(dataChrh, lastposhAdd);
    }else{
      #first chr
    }

  }
  l<-l+lastposlAdd;
  h<-h+lastposhAdd;
  if(data[i,2]==1 ){
    dataChrl<-c(dataChrl, l);
  }

#  print("l");
#  print(c);
#  print(l);
#  print(h);
#  print(lastposl);
#  print(lastposh);
#  print(lastposlAdd);
#  print(lastposhAdd);
  dataN<-rbind(dataN,c( (l+h)/2, data[i,4] )); #coverage
  lastposl<-l;
  lastposh<-h;
  lastposc<-as.character(c);  
}
dataChr<-c(dataChr, lastposc);
#dataChrl<-c(dataChrl, lastposlAdd);
lastposhAdd<-lastposh;

dataChrh<-c(dataChrh, lastposhAdd);
pdf(args[2] ,width=14);

plot(dataN[,1],dataN[,2],xaxt='n', ann=FALSE,pch="*",col="darkred");

for(i in 1:length(dataChrl)){
  abline(v=dataChrl[i],lty=3);

}

axis(1, at=((dataChrl+dataChrh)/2), labels=dataChr,las=2)

dev.off();
