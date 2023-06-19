// VowelRecognition1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include"Configuration.h"
#include"DataProcessing.h"
#include<stdlib.h>
#include "LinearPredictiveCoding.h"
#include<string>
#define p 12
#define q 12

ld tokhuraWeight[] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};
char vowels[] = {'A','E','I','O','U'};
void testVowel(FILE *input) {

	/*Reading test C vector*/
	ld testC[5][12];
	for(int i=0;i<5;i++)
		for(int j=0;j<p;j++)
			fscanf(input,"%Lf",&testC[i][j]);

	/*Reading codevector in an array*/
	FILE *codeBookFP = fopen("codeBook.txt","r");
	/*ld** codeBook = (ld**)malloc(5 * sizeof(ld*));
	for(int i=0;i<5;i++)
		codeBook[i] = (ld *)malloc((p+1)*sizeof(ld));*/
	ld codeBook[5][13];
	for(int i=0;i<5;i++)
		for(int j=0;j<p;j++)
			fscanf(codeBookFP,"%Lf",&codeBook[i][j]);

	/*Calculating distance for each row in codebook*/
	ld distance[5] = {0.0};
	for(int i=0;i<5;i++) {
		distance[i] = 0.0;
		for(int k=0;k<5;k++) {
			ld distanceTemp = 0.0;
			for(int j=0;j<p;j++) {
				ld temp1 = testC[k][j]-codeBook[i][j];
				temp1*=temp1;
				distanceTemp+=tokhuraWeight[j]*temp1;
			}
			distance[i]+=distanceTemp;
			//distance[i]+=tokhuraWeight[i]*(testC[j]-codeBook[i][j]);
		}
		distance[i]/=5.000000;
		printf("\ndistance[%d] = %Lf",i,distance[i]);
	}

	/*identifying character*/
	ld minDistance = distance[0];
	char result = 'A';
	for(int i=1;i<5;i++) {
		if(minDistance>distance[i]) {
			minDistance = distance[i];
			result = vowels[i];
		}
	}

	/*printing result*/
	printf("\nThe test vowel is %c\n",result);

	//clearing resources
	fclose(codeBookFP);
	/*for (int i = 0; i < 5; i++)
		free(codeBook[i]);
	free(codeBook);*/
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*Calculating DC shift*/
	ld offset = computeDCShift();
	printf("\nComputed offset: %lf",offset);

	
	
	//Code book training do it for each vowel
	for(int vowelNum=0;vowelNum<5;vowelNum++) {
		char fileName[500];
		sprintf(fileName, "VowelTest_%c.txt",vowels[vowelNum]);	
		FILE *c_out = fopen(fileName,"w");
		for(int fileNumItr = 1;fileNumItr<=10;fileNumItr++) {
			sprintf(fileName, "Dataset\\Vowels\\224101034_%c_%d.txt",vowels[vowelNum],fileNumItr);	
			FILE *fp = fopen(fileName,"r"); 
		
			//Calculating normalization factor
			ld normalizationFactor = computeNormalizationFactor(fp);
			//printf("\nComputed normalization factor: %Lf",normalizationFactor);
	
			//Compute energy and ZCR
			rewind(fp);
			FILE *EnergyZcrTable = fopen("EnergyZcrTable.txt", "w");
			int frameNumber;
			frameNumber = computeEnergyZcr(fp,EnergyZcrTable,offset,normalizationFactor);//ignoring first 3 frames
			fclose(EnergyZcrTable);

			//for convenience we are copying the energy and zcr values in array
			EnergyZcrTable = fopen("EnergyZcrTable.txt", "r");
			ld energyArr[500] = {0.0};//assuming maximum frame number is 499
			ld zcrArr[500] = {0.0};//assuming maximum frame number is 499
			long totaLframes = 0;
			while(!feof (EnergyZcrTable)) {
				int currentFrame;
				ld energy,zcr;
				fscanf(EnergyZcrTable,"%d",&currentFrame);//frameNumber
				fscanf(EnergyZcrTable,"%Lf",&energy);//energy
				energyArr[currentFrame] = energy;
				fscanf(EnergyZcrTable,"%Lf",&zcr);//zcr
				zcrArr[currentFrame] = zcr;
			}
	

			//Detect 5 steady frames
			ld curMax = 0;
			int curIndex = 0;
			for(int i=0;i<=frameNumber;i++) {
				if(energyArr[i]>curMax) {
					curMax = energyArr[i];
					curIndex = i;
				}
			
			}
			ld steadyFrames[5][frameSize+1];
			rewind(fp);
			int curFrame = 1;
			int startSteadyFrame = curIndex - 2;
			while(curFrame<startSteadyFrame) {
				for(int i=0;i<frameSize;i++) {
					ld temp;
					fscanf(fp,"%Lf",&temp);
				}
				curFrame++;
			}
			for(int i = 0;i < 5;i++) {
				for(int j=0;j<frameSize;j++) {
					ld temp;
					fscanf(fp,"%Lf",&temp);
					steadyFrames[i][j+1] = temp;
				}
			}

			//calculating c for each steady frame
			for(int i = 0;i < 5;i++)
				calculateC(steadyFrames[i],c_out);


			//clearing resource
			fclose(EnergyZcrTable);
			fclose(fp);
			

			remove("EnergyZcrTable.txt");
		}
		fclose(c_out);
	}



	//generating codebook
	FILE *codeBookfp = fopen("codeBook.txt","w"); 
	
	for(int vowelNum=0;vowelNum<5;vowelNum++) {
		char fileName[400];
		sprintf(fileName, "vowelTest_%c.txt",vowels[vowelNum]);	
		FILE *fp = fopen(fileName,"r"); 
		ld codeBookValue[12];//for each value we shall store the c values in this array
		for(int j=0;j<12;j++) codeBookValue[j] = 0;
		for(int i = 0;i<50;i++) {
			for(int j=0;j<12;j++) {
				ld temp;
				fscanf(fp,"%Lf",&temp);
				codeBookValue[j]+=temp;
			}
		}
		fclose(fp);
		for(int j=0;j<12;j++) {
			codeBookValue[j]/=50.000000;
			fprintf(codeBookfp,"%Lf ",codeBookValue[j]);
		}
		fprintf(codeBookfp,"\n");
	}
	fclose(codeBookfp);


	
	//testing
	for(int vowelNum=0;vowelNum<5;vowelNum++) {
		for(int fileNumItr = 11;fileNumItr<=20;fileNumItr++) {
			char fileName[500];
			sprintf(fileName, "Dataset\\Vowels\\224101034_%c_%d.txt",vowels[vowelNum],fileNumItr);	
			FILE *fp = fopen(fileName,"r"); 
			printf("\nTesting %s",fileName);
			//Calculating normalization factor
			ld normalizationFactor = computeNormalizationFactor(fp);
			printf("\nComputed normalization factor: %Lf",normalizationFactor);
	
			//Compute energy and ZCR
			rewind(fp);
			FILE *out = fopen("Out.txt", "w");
			int frameNumber;
			frameNumber = computeEnergyZcr(fp,out,offset,normalizationFactor);//ignoring first 3 frames
			fclose(out);

			//for convenience we are copying the energy and zcr values in array
			out = fopen("Out.txt", "r");
			ld energyArr[500] = {0.0};//assuming maximum frame number is 499
			ld zcrArr[500] = {0.0};//assuming maximum frame number is 499
			long totaLframes = 0;
			while(!feof (out)) {
				int currentFrame;
				ld energy,zcr;
				fscanf(out,"%d",&currentFrame);//frameNumber
				fscanf(out,"%Lf",&energy);//energy
				energyArr[currentFrame] = energy;
				fscanf(out,"%Lf",&zcr);//zcr
				zcrArr[currentFrame] = zcr;
			}
	

			//Detect 5 steady frames
			ld curMax = 0;
			int curIndex = 0;
			for(int i=0;i<=frameNumber;i++) {
				if(energyArr[i]>curMax) {
					curMax = energyArr[i];
					curIndex = i;
				}
			
			}
			ld steadyFrames[5][frameSize+1];
			rewind(fp);
			int curFrame = 1;
			int startSteadyFrame = curIndex - 2;
			while(curFrame<startSteadyFrame) {
				for(int i=0;i<frameSize;i++) {
					ld temp;
					fscanf(fp,"%Lf",&temp);
				}
				curFrame++;
			}
			for(int i = 0;i < 5;i++) {
				for(int j=0;j<frameSize;j++) {
					ld temp;
					fscanf(fp,"%Lf",&temp);
					steadyFrames[i][j+1] = temp;
				}
			}

			//calculating c for each steady frame
			FILE *c_out = fopen("test.txt","w");
			for(int i = 0;i < 5;i++)
				calculateC(steadyFrames[i],c_out);

			fclose(c_out);
			c_out = fopen("test.txt","r");
			testVowel(c_out);

			//clearing resource
			fclose(c_out);
			fclose(out);
			fclose(fp);
			

			remove("Out.txt");
			remove("test.txt");

		}
	}

	
	system("pause");
	return 0;
}

