#include"DataProcessing.h"
#include<cstdio>
#include<algorithm>

ld computeDCShift() {
	char inputFileName[100];
	ld temp,Energy = 0,sampleNumber=0;
	FILE *fp = fopen("DC_shift.txt", "r");//recorded this file in silence to capture DC shift

	if(!fp) {
		printf("Sorry input file could not be opened\n");
		return 0;
	}

	while (!feof (fp)) {  
		fscanf (fp, "%Lf", &temp); 
		Energy+=temp;
		sampleNumber++;
	}
	Energy/=sampleNumber; //calculating avg energy as dc shift value
    fclose(fp);
	return Energy;
}


ld computeNormalizationFactor(FILE *fp) {
	ld maxEnergy;
	if(!feof (fp))
		fscanf (fp, "%Lf", &maxEnergy); 
	while (!feof (fp)){
		ld temp;
		fscanf (fp, "%Lf", &temp); 
		maxEnergy = std::max(maxEnergy,abs(temp));
	}
	ld factor = 5000.000000/(maxEnergy*1.0);  
	/*if(factor>1)//this can happen when our data is already below 5000. In this case, as the data is already in range hence no need to normalize
		return 1;*/
	return factor;
}

int computeEnergyZcr(FILE *ip,FILE *op,ld offset,ld factor) {
	int finalFrameNumber=0;
	int frameNumber=0;
	ld Energy =0,zcrCount=0;
	int index = 0;
	short flag = 0;
	//fprintf(op, "Frame Number,Energy,ZCR\n"); //printing the heading
	
	while (!feof (ip)) {  //calculating energy,zcr and frame number till the end of the file.
		frameNumber++;
		Energy = 0;
		zcrCount = 0;
		flag = 0;
		int i=0;
		for(i=0;i<frameSize;i++) {   //calculate energy and ZCR from frame number 3. Ignoring first 3 frames as those are mainly silence.
			if(feof (ip)) break;   //if eof before 320 samples in that frame then leave the last frame
			ld temp;
			fscanf (ip, "%Lf", &temp);
			temp = (temp-offset)*factor;   //data cleaning
			Energy+=temp*temp;
			if(i>0) {
				if((flag==1 && temp<0) || (flag==-1 && temp>0))
					zcrCount++;
			}
			if(temp>0)
				flag=1;
			if(temp<0)
				flag=-1;
		}
		if(i>=frameSize && frameNumber>=4) {//throw the last frame if i<frameSize i.e. last frame contains less than 320 samples
			//ignoring first 3 frames(320*3 samples) as they majorly contain silence
			Energy/=frameSize;
			fprintf(op, "%d %Lf %Lf\n", frameNumber,Energy,zcrCount); 
			finalFrameNumber = frameNumber;	
		}
		

	}
	return finalFrameNumber;
}


