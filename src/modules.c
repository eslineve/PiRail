#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <string.h>

#include "./../lib/system.h"

#include "./../lib/rail.h"
#include "./../lib/switch.h"
#include "./../lib/signals.h"
#include "./../lib/trains.h"

#include "./../lib/modules.h"

#include "./../lib/algorithm.h"

struct Unit *Units[MAX_Modules];

void setup_JSON(int arr[], int arr2[], int size, int size2){
	char setup_data[100];

	setup_data[0] = 2;
	int setup_data_l = 2 + size + size2;

	int i = 2;

	for(i;(i-2)<size;i++){
		setup_data[i] = arr[i-2];
	}

	if(size2 != 0){
		setup_data[1] = size;

		for(i;(i-2-size)<size2;i++){
			setup_data[i] = arr2[i-2-size];
		}
	}
}

void Create_Unit(int Module){
	struct Unit *Z = (struct Unit*)malloc(sizeof(struct Unit));

	Units[Module] = Z;
}

void Create_Unit2(int Module,int OUT,int IN){
	struct Unit *Z = (struct Unit*)malloc(sizeof(struct Unit));

	Units[Module] = Z;

	struct Rail_link ** A = (struct Rail_link**)calloc( IN,sizeof(struct Rail_link));
	struct Rail_link ** B = (struct Rail_link**)calloc(OUT,sizeof(struct Rail_link));

	Z->B_L = 8;
	Z->B = (struct Seg **)calloc(Z->B_L,sizeof(struct Seg *));

	Z->S_L = 8;
	Z->S = (struct Swi **)calloc(Z->S_L,sizeof(struct Swi *));

	for(int i = 0;i<8;i++){
		Z->M[i] = 0;
	}
	for(int i = 0;i<32;i++){
		Z->Signals[i] = 0;
	}

	Z->In_length = IN;
	Z->In = A;
	Z->Out_length = OUT;
	Z->Out = B;
	IN--;OUT--;//To make the division round down;
	Z->InRegs    = (uint8_t *)malloc(( IN/8)+1);
	Z->OutRegs   = (uint8_t *)malloc((OUT/8)+1);
	Z->BlinkMask = (uint8_t *)malloc((OUT/8)+1);
}

void join(struct SegC Adr, struct SegC link){
	printf("LINK %c%i:%i => %c%i:%i\t",Adr.type,Adr.Module,Adr.Adr,link.type,link.Module,link.Adr);
	if(Adr.type == 'R' && Units[Adr.Module]->B[Adr.Adr]){
		Units[Adr.Module]->B[Adr.Adr]->PrevC = link;
		printf("D\n");
	}else if(Adr.type == 'S'){
		Units[Adr.Module]->S[Adr.Adr]->AppC = link;
	}else if(Adr.type == 's'){
		if(Adr_Comp2(Units[Adr.Module]->S[Adr.Adr]->DivC, EMPTY_BL())){
			Units[Adr.Module]->S[Adr.Adr]->DivC = link;
		}else{
			Units[Adr.Module]->S[Adr.Adr]->StrC = link;
		}
	}
}

void clear_Modules(){

	for(int i = 0;i<MAX_Modules;i++){
	  if(Units[i]){
	  	printf("Clearing module %i\n",i);
	    //clear Rail_link
	    printf("- Rail link out\n");
	    free(Units[i]->Out);
	    Units[i]->Out = NULL;
	    printf("- Rail link in\n");
	    free(Units[i]->In);
	    Units[i]->In = NULL;

	    //clear Segments
	    for(int j = 0;j<=Units[i]->B_nr;j++){
	      printf("- Block %i\n",j);
	      free(Units[i]->B[j]);
	      Units[i]->B[j] = NULL;
	    }
	    //clear Switches
	    for(int j = 0;j<=Units[i]->Swi_nr;j++){
	    	printf("- Switch %i\n",j);
	      free(Units[i]->S[j]);
	      Units[i]->S[j] = NULL;
	    }
	    //clear Mods
	    for(int j = 0;j<=Units[i]->Mod_nr;j++){
	    	printf("- Mod %i\n",j);
	      free(Units[i]->M[j]);
	      Units[i]->M[j] = NULL;
	    }
	    //clear Signals
	    for(int j = 0;j<=Units[i]->Signal_nr;j++){
	    	printf("- Signal %i\n",j);
	      free(Units[i]->Signals[j]);
	      Units[i]->Signals[j] = NULL;
	    }
	    //clear Stations
	    for(int j = 0;j<=Units[i]->Station_nr;j++){
	    	printf("- Station %i\n",j);
	      free(Units[i]->St[j]);
	      Units[i]->St[j] = NULL;
	    }

	    printf("- Unit %i\n",i);
	    free(Units[i]);
	    Units[i] = 0;
	    printf("\t Cleared!\n");
	  }
	}
}

struct link Modules(int m, struct link IN){
	//Loop Left
	struct link link;
	link.Adr3 = EMPTY_BL();
	link.Adr4 = EMPTY_BL();

	int Seg_i = 0;
	int Swi_i = 0;
	int Sig_i = 0;

	if(m == 1){
		Create_Unit2(m,8,8);

		/*			,-1--.
					 |     `--0--
		1 0    \-.--2---3--
		        ||

		Switch
		0		0--1
		1   2--3
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 3;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);
		join(IN.Adr1,CAdr(m,1,'s'));
		join(IN.Adr2,CAdr(m,0,'s'));

		Create_Segment(0,CAdr(m,0,'R'),CAdr(m,1,'R'),EMPTY_BL(),     speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),CAdr(m,1,'S'),CAdr(m,0,'R'),speed_A,0,2,100);
 		Create_Segment(2,CAdr(m,2,'R'),CAdr(m,0,'S'),CAdr(m,3,'R'),speed_A,0,1,100);
 		Create_Segment(3,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL(),     speed_A,0,1,100);

		//LINK
		Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),IN.Adr1,(int [2]){2,3},1);
		Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),IN.Adr2,(int [2]){0,1},0);

		Units[m]->S[0]->Detection_Block = Units[m]->B[2];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
	}
	else if(m == 2){
		Create_Unit2(m,8,8);

		/*			,--1-.
			--0--'     |
			--3---2-.-/					0 1
							||
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 1;link.Adr1.type = 's';
		link.Adr2.Module = m;link.Adr2.Adr = 0;link.Adr2.type = 's';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,3,'R'));

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,1,'R'),speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,1,'S'),speed_A,0,2,100);
 		Create_Segment(2,CAdr(m,2,'R'),CAdr(m,3,'R'),CAdr(m,0,'S'),speed_A,0,1,100);
 		Create_Segment(3,CAdr(m,3,'R'),IN.Adr2,CAdr(m,2,'R'),speed_A,0,1,100);

		Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),EMPTY_BL(),(int [2]){0,1},1);
		Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),EMPTY_BL(),(int [2]){2,3},1);

		Units[m]->S[0]->Detection_Block = Units[m]->B[2];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
	}
	else if(m == 3){//Station 4 bakken
		Create_Unit2(m,8,8);
		/*
		Blocks address (Octal)
		  				 /-- -10-11- --\
		--0-- -\1-'--- -12-13- ---`-30-/- -31-
		--2-- -3\-,--- -14-15- ---,-32-'- -33-
							 \-- -16-17- --/
							  \- -20-21- -/

		Block numbers (Decimal)
		  				 /-- --2--3- --\
		--0-- -\1-'--- --4--5- ---`--6-/- --7-
		--8-- -9\-,--- -10-11- ---,-16-'- -17-
							 \-- -12-13- --/
							  \- -14-15- -/

		Switches numbers (Decimal)
				0 1										5 6
				 2 3								 7 8
					  4							  9
 		Switches addresses (octal)
		0	 0--1			5 20-21
		1  2--3			6 22-23
		2  4--5			7 24-25
		3  6--7			8 26-27
		4 10-11			9 30-31
		*/
		link.Adr1.Module = m;link.Adr1.Adr =  7;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 17;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,8,'R'));

		Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
		Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,4,'R') ,(int [2]){002,003},1);			//Switch 1
		Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,8,'R') ,(int [2]){004,005},0);	//Switch 2
		Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,10,'R'),(int [2]){006,007},1); //Switch 3
		Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,12,'R'),CAdr(m,14,'R'),(int [2]){010,011},1);		//Switch 4
		Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,3,'R') ,CAdr(m,5,'R') ,(int [2]){020,021},1);				//Switch 010
		Create_Switch(CAdr(m,6,2),CAdr(m,7,'R'),CAdr(m,8,'s') ,CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
		Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,11,'R'),(int [2]){024,025},1); //Switch 013
		Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,17,'R'),(int [2]){026,027},0); //Switch 014
		Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,13,'R'),CAdr(m,15,'R'),(int [2]){030,031},1);     //Switch 012

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
		Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,100);

		Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'s'),CAdr(m,3,'R')  ,speed_C,0,0,50);
		Create_Segment(011,CAdr(m,3,'R'),CAdr(m,2,'R'),CAdr(m,5,'s'),speed_C,0,0,50);
		Create_Segment(012,CAdr(m,4,'R'),CAdr(m,1,'s'),CAdr(m,5,'R'),  speed_C,0,0,50);
		Create_Segment(013,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(030,CAdr(m,6,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
		Create_Segment(031,CAdr(m,7,'R'),CAdr(m,6,'S'),EMPTY_BL(),speed_B,0,0,100);


		Create_Segment(2,CAdr(m,8,'R'),EMPTY_BL(),CAdr(m,2,'s'),speed_B,0,1,50);
		Create_Segment(3,CAdr(m,9,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);

		Create_Segment(014,CAdr(m,10,'R'),CAdr(m,3,'s') ,CAdr(m,11,'R'),speed_C,0,0,50);
		Create_Segment(015,CAdr(m,11,'R'),CAdr(m,10,'R'),CAdr(m,7,'s') ,speed_C,0,0,50);
		Create_Segment(016,CAdr(m,12,'R'),CAdr(m,4,'s') ,CAdr(m,13,'R'),speed_C,0,0,50);
		Create_Segment(017,CAdr(m,13,'R'),CAdr(m,12,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);
		Create_Segment(020,CAdr(m,14,'R'),CAdr(m,4,'s') ,CAdr(m,15,'R'),speed_C,0,0,50);
		Create_Segment(021,CAdr(m,15,'R'),CAdr(m,14,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);

		Create_Segment(032,CAdr(m,16,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
		Create_Segment(033,CAdr(m,17,'R'),CAdr(m,8,'s'),EMPTY_BL(),speed_B,0,1,50);
	}
	else if(m == 4){//Station 4 bakken
		Create_Unit2(m,8,32);
		/*
		Blocks addresses (Octal)
		  				 /-- -10-11- -30-31- --\
		--0-- -\1-'--- -12-13- -32-33- ---`-50-/- -51-
		--2-- -3\-,--- -14-15- -34-35- ---,-52-'- -53-
							 \-- -16-17- -36-37- --/
							  \- -20-21- -40-41- -/

		Blocks addresses (Decimal)
		  				  /-- --2--3- --4--5- --\
		--0-- --\1-'--- --6--7- --8--9- ---`-10-/- -11-
		-12-- -13\-,--- -14-15- -16-17- ---,-26'-- -27-
							  \-- -18-19- -20-21- --/
							   \- -22-23- -24-25- -/


		Switches numbers (Decimal)
				0 1										5 6
				 2 3								 7 8
					  4							  9
 		Switches addresses (octal)
		0	 0--1			5 20-21
		1  2--3			6 22-23
		2  4--5			7 24-25
		3  6--7			8 26-27
		4 10-11			9 30-31
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 11;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 27;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,12,'R'));

		Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
		Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,6,'R') ,(int [2]){002,003},1);			//Switch 1
		Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,12,'R'),(int [2]){004,005},0);	//Switch 2
		Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,14,'R'),(int [2]){006,007},1); //Switch 3
		Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,18,'R'),CAdr(m,22,'R'),(int [2]){010,011},1);		//Switch 4
		Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,5,'R') ,CAdr(m,9,'R') ,(int [2]){020,021},1);				//Switch 010
		Create_Switch(CAdr(m,6,2),CAdr(m,11,'R'),CAdr(m,8,'s'),CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
		Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,17,'R'),(int [2]){024,025},1); //Switch 013
		Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,27,'R'),(int [2]){026,027},0); //Switch 014
		Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,21,'R'),CAdr(m,25,'R'),(int [2]){030,031},1);     //Switch 012

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
		Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,100);

		Create_Segment(010,CAdr(m,2,'S'),CAdr(m,1,'s'),CAdr(m,3,'R'),speed_C,0,0,50);
		Create_Segment(011,CAdr(m,3,'S'),CAdr(m,2,'R'),CAdr(m,4,'R'),speed_C,0,0,50);
		Create_Segment(030,CAdr(m,4,'S'),CAdr(m,3,'R'),CAdr(m,5,'R'),speed_C,0,0,50);
		Create_Segment(031,CAdr(m,5,'S'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(012,CAdr(m,6,'S'),CAdr(m,1,'s'),CAdr(m,7,'R'),speed_C,0,0,50);
		Create_Segment(013,CAdr(m,7,'S'),CAdr(m,6,'R'),CAdr(m,8,'R'),speed_C,0,0,50);
		Create_Segment(032,CAdr(m,8,'S'),CAdr(m,7,'R'),CAdr(m,9,'R'),speed_C,0,0,50);
		Create_Segment(033,CAdr(m,9,'S'),CAdr(m,8,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(050,CAdr(m,10,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
		Create_Segment(051,CAdr(m,11,'R'),CAdr(m,6,'S'),EMPTY_BL(),speed_B,0,0,100);


		Create_Segment(2,CAdr(m,12,'R'),IN.Adr2,CAdr(m,2,'s'),speed_B,0,1,50);
		Create_Segment(3,CAdr(m,13,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);

		Create_Segment(014,CAdr(m,14,'S'),CAdr(m,3,'s') ,CAdr(m,15,'R'),speed_C,0,1,50);
		Create_Segment(015,CAdr(m,15,'S'),CAdr(m,14,'R'),CAdr(m,16,'R'),speed_C,0,1,50);
		Create_Segment(034,CAdr(m,16,'S'),CAdr(m,15,'R'),CAdr(m,17,'R'),speed_C,0,1,50);
		Create_Segment(035,CAdr(m,17,'S'),CAdr(m,16,'R'),CAdr(m,7 ,'s'),speed_C,0,1,50);

		Create_Segment(016,CAdr(m,18,'S'),CAdr(m,4,'s') ,CAdr(m,19,'R'),speed_C,0,1,50);
		Create_Segment(017,CAdr(m,19,'S'),CAdr(m,18,'R'),CAdr(m,20,'R'),speed_C,0,1,50);
		Create_Segment(036,CAdr(m,20,'S'),CAdr(m,19,'R'),CAdr(m,21,'R'),speed_C,0,1,50);
		Create_Segment(037,CAdr(m,21,'S'),CAdr(m,20,'R'),CAdr(m,9,'s') ,speed_C,0,1,50);

		Create_Segment(020,CAdr(m,22,'S'),CAdr(m,4,'s'),CAdr(m,23,'R'),speed_C,0,1,50);
		Create_Segment(021,CAdr(m,23,'S'),CAdr(m,22,'R'),CAdr(m,24,'R'),speed_C,0,1,50);
		Create_Segment(040,CAdr(m,24,'S'),CAdr(m,23,'R'),CAdr(m,25,'R'),speed_C,0,1,50);
		Create_Segment(041,CAdr(m,25,'S'),CAdr(m,24,'R'),CAdr(m,9,'s'),speed_C,0,1,50);

		Create_Segment(052,CAdr(m,26,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);
		Create_Segment(053,CAdr(m,27,'R'),CAdr(m,8,'s'),EMPTY_BL(),speed_B,0,1,50);

		Units[m]->S[0]->Detection_Block = Units[m]->B[1];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
		Units[m]->S[2]->Detection_Block = Units[m]->B[13];
		Units[m]->S[3]->Detection_Block = Units[m]->B[13];
		Units[m]->S[4]->Detection_Block = Units[m]->B[13];

		Units[m]->S[5]->Detection_Block = Units[m]->B[10];
		Units[m]->S[6]->Detection_Block = Units[m]->B[10];
		Units[m]->S[7]->Detection_Block = Units[m]->B[26];
		Units[m]->S[8]->Detection_Block = Units[m]->B[26];
		Units[m]->S[9]->Detection_Block = Units[m]->B[26];

		/*Linking Switches*/
		 struct L_Swi_t * B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));
		 struct SegC ADR = CAdr(m,0,'S');
		 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Units[m]->S[2]->L_Swi[0] = B_Swi;
		 Units[m]->S[6]->L_Swi[0] = B_Swi;
		 Units[m]->S[8]->L_Swi[0] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

		 ADR.Adr = 2;
		 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Units[m]->S[0]->L_Swi[0] = B_Swi;
		 Units[m]->S[6]->L_Swi[1] = B_Swi;
		 Units[m]->S[8]->L_Swi[1] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

	 	 ADR.Adr = 6;
	 	 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Units[m]->S[0]->L_Swi[1] = B_Swi;
		 Units[m]->S[2]->L_Swi[1] = B_Swi;
		 Units[m]->S[8]->L_Swi[2] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

	 	 ADR.Adr = 8;
	 	 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Units[m]->S[0]->L_Swi[2] = B_Swi;
		 Units[m]->S[2]->L_Swi[2] = B_Swi;
		 Units[m]->S[6]->L_Swi[2] = B_Swi;
		/**/
		//
		/*Setting Switch preferences*/
			struct P_Swi_t * P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
			P->type = 0; 		//Always
			P->state = 0;		//Straigth when approaching switch
			Units[m]->S[8]->pref[0] = P;
			P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
			P->type = 0; 		//Always
			P->state = 1;		//Diverging when approaching switch
			Units[m]->S[2]->pref[0] = P;
		//
		/*Stations*/
			Create_Station(m,"Spoor 1",1,4,(int [4]){2,3,4,5});
			Create_Station(m,"Spoor 2",1,4,(int [4]){6,7,8,9});
			Create_Station(m,"Spoor 3",1,4,(int [4]){14,15,16,17});
			Create_Station(m,"Spoor 4",1,4,(int [4]){18,19,20,21});
			Create_Station(m,"Spoor 5",1,4,(int [4]){22,23,24,25});
		//
		/*Signals*/
			//All type 2 signals
			//NSignals

			uint8_t adr[3] = {0,1,2};
			struct Seg ** block_id = Units[m]->B;
			char b[BLOCK_STATES] = {1,2,4}; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			char c[BLOCK_STATES] = {1,0,0};

			create_signal2(block_id[2],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 3;adr[1] = 4;adr[2] = 5;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(block_id[6],3,adr,b,c,0);


			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 16;adr[1] = 17;adr[2] = 18;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(block_id[17],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 19;adr[1] = 20;adr[2] = 21;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(block_id[21],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 22;adr[1] = 23;adr[2] = 24;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(block_id[25],3,adr,b,c,0);

			//PSignals

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 7;adr[1] = 8;adr[2] = 9;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(block_id[14],3,adr,b,c,1);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 10;adr[1] = 11;adr[2] = 12;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(block_id[18],3,adr,b,c,1);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 13;adr[1] = 14;adr[2] = 15;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(block_id[22],3,adr,b,c,1);
		//
		/*One Way*/
			Units[m]->B[0]->oneWay = TRUE;
			Units[m]->B[11]->oneWay = TRUE;
			Units[m]->B[12]->oneWay = TRUE;
			Units[m]->B[27]->oneWay = TRUE;
		//

		Units[m]->B[1]->dir  = 0;
		Units[m]->B[10]->dir = 0;
		Units[m]->B[13]->dir  = 1;
		Units[m]->B[26]->dir = 1;

	}/*
	else if(m == 5){//T piece

		Create_Unit(m);

		join(IN.Adr1,C_Adr(m,1,1));
		Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,END_BL,speed_A,0,0,100);

		C_Seg(Seg_i++,C_Adr(m,2,0),0);
		C_Seg(Seg_i++,C_Adr(m,5,0),0);

		join(IN.Adr2,C_AdrT(m,2,1,'S'));
		Create_Switch(Swi_i++,C_Adr(m,2,1),IN.Adr2,C_Adr(m,3,1),C_AdrT(m,5,1,'s'),1);
		Create_Switch(Swi_i++,C_Adr(m,5,1),END_BL,C_Adr(m,4,1),C_AdrT(m,2,1,'s'),1);

		Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,2,1,'s'),END_BL,speed_A,0,1,50);
		Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,5,1,'s'),END_BL,speed_A,0,2,50);

		blocks[m][2][0]->dir  = 1;
		blocks[m][5][0]->dir = 1;

		link.Adr1 = C_Adr(m,1,1);
		link.Adr2 = C_AdrT(m,5,1,'S');
		link.Adr3 = C_AdrT(m,4,1,'R');
		link.Adr4 = C_AdrT(m,3,1,'R');
	}
	else if(m == 6){//Rangeer Brug
		Create_Unit(m);
		join(IN.Adr1,C_Adr(m,1,1));
		Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,C_AdrT(m,3,1,'s'),speed_A,0,0,90);

		join(IN.Adr2,C_Adr(m,2,1));
		Create_Segment(Seg_i++,C_Adr(m,2,1),IN.Adr2,C_AdrT(m,3,1,'s'),speed_A,0,1,90);

		C_Seg(Seg_i++,C_Adr(m,3,0),0);

		Create_Switch(Swi_i++,C_Adr(m,3,1),END_BL,C_Adr(m,2,1),C_Adr(m,1,1),1);

		blocks[m][1][1]->oneWay = TRUE;
		blocks[m][2][1]->oneWay = TRUE;

		link.Adr1 = C_AdrT(m,3,1,'S');
		link.Adr2 = END_BL;
	}
	else if(m == 7){//Rangeer
		Create_Unit(m);

		C_Seg(Seg_i++,C_Adr(m,1,0),0);

		Create_Switch(Swi_i++,C_Adr(m,1,1),C_AdrT(m,1,2,'s'),C_Adr(m,3,1),C_Adr(m,2,1),1);
		Create_Switch(Swi_i++,C_Adr(m,1,2),C_AdrT(m,1,3,'m'),C_Adr(m,4,1),C_AdrT(m,1,1,'S'),1);

		join(IN.Adr1,C_AdrT(m,1,3,'M'));
		struct adr A[10] = {{m,1,2,'S'},{m,5,1,'R'},{m,1,4,'S'},END_BL};
		struct adr B[10] = {link.Adr1,link.Adr1,link.Adr1,END_BL};
		Create_Moduls(Swi_i++,C_Adr(m,1,3),A,B,3);
		//Create_Switch(Swi_i++,C_Adr(m,1,3),link.Adr1,C_AdrT(m,1,2,'S'),C_AdrT(m,1,4,'S'),1);
		//Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'s'),C_AdrT(m,1,5,'S'),C_Adr(m,5,1),1);

		Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'m'),C_Adr(m,6,1),C_AdrT(m,1,5,'S'),1);
		Create_Switch(Swi_i++,C_Adr(m,1,5),C_AdrT(m,1,4,'s'),C_Adr(m,7,1),C_Adr(m,8,1),1);

		Create_Segment(Seg_i++,C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,4),C_Adr(m,2,3),C_Adr(m,2,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,5),C_Adr(m,2,4),C_Adr(m,2,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,6),C_Adr(m,2,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,4),C_Adr(m,3,3),C_Adr(m,3,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,5),C_Adr(m,3,4),C_Adr(m,3,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,6),C_Adr(m,3,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,1,2,'s'),C_Adr(m,4,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,2),C_Adr(m,4,1),C_Adr(m,4,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,3),C_Adr(m,4,2),C_Adr(m,4,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,4),C_Adr(m,4,3),C_Adr(m,4,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,5),C_Adr(m,4,4),C_Adr(m,4,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,6),C_Adr(m,4,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,5,1),C_AdrT(m,1,3,'m'),C_Adr(m,5,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,2),C_Adr(m,5,1),C_Adr(m,5,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,3),C_Adr(m,5,2),C_Adr(m,5,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,4),C_Adr(m,5,3),C_Adr(m,5,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,5),C_Adr(m,5,4),C_Adr(m,5,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,6),C_Adr(m,5,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,6,1),C_AdrT(m,1,4,'s'),C_Adr(m,6,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,2),C_Adr(m,6,1),C_Adr(m,6,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,3),C_Adr(m,6,2),C_Adr(m,6,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,4),C_Adr(m,6,3),C_Adr(m,6,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,5),C_Adr(m,6,4),C_Adr(m,6,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,6),C_Adr(m,6,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,7,1),C_AdrT(m,1,5,'s'),C_Adr(m,7,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,4),C_Adr(m,7,3),C_Adr(m,7,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,5),C_Adr(m,7,4),C_Adr(m,7,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,6),C_Adr(m,7,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,8,1),C_AdrT(m,1,5,'s'),C_Adr(m,8,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,4),C_Adr(m,8,3),C_Adr(m,8,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,5),C_Adr(m,8,4),C_Adr(m,8,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,6),C_Adr(m,8,5),END_BL,speed_A,0,0,50);

		link.Adr1 = END_BL;
		link.Adr2 = END_BL;
	}*/
	else if(m == 8 || m == 9 || m == 10 || m == 12 || m == 13 || m == 14){//Stad
		Create_Unit2(m,8,8);
		/*
		Block addresses (Octal)
		--00--02--010--012--
		--01--03--011--013--

		Block numbers (Decimal)
		--0--1--2--3--
		--4--5--6--7--
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 3;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 7;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,4,'R'));

		Create_Segment(0  ,CAdr(m,0,'R'),IN.Adr1      ,CAdr(m,1,'R'),speed_A,0,0,100);
		Create_Segment(02 ,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,2,'R'),speed_A,0,0,100);
		Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'R'),CAdr(m,3,'R'),speed_A,0,0,100);
		Create_Segment(012,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL()   ,speed_A,0,0,100);

		Create_Segment(1  ,CAdr(m,4,'R'),IN.Adr2      ,CAdr(m,5,'R'),speed_A,0,1,100);
		Create_Segment(03 ,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,6,'R'),speed_A,0,1,100);
		Create_Segment(011,CAdr(m,6,'R'),CAdr(m,5,'R'),CAdr(m,7,'R'),speed_A,0,1,100);
		Create_Segment(013,CAdr(m,7,'R'),CAdr(m,6,'R'),EMPTY_BL()   ,speed_A,0,1,100);
	}
	else if(m == 11){//Korte Bocht
		Create_Unit2(m,8,8);

		link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 1;link.Adr2.type = 'R';

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,1,'R'));

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,EMPTY_BL(),speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),IN.Adr2,EMPTY_BL(),speed_A,0,1,100);
	}
	return link;
}

void LoadModules(int M){
	if(M == 0){
		return;
	}

	printf("Load module %i\n",M);

	if(M != 4 && M != 8 && M != 1 && M != 2){
		return; //Function is not ready
	}

	//Try to open file
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	char folder[]   = "./modules/";
	char filename[] = "/prop.txt";
	char file[30] = "";

	char nr[3] = "";
	sprintf(nr, "%d", M);

	strcat(file,folder);
	strcat(file,nr);
	strcat(file,filename);

	fp = fopen(file, "r");
	if (fp == NULL){
        printf("Failed to open File: %s\n",file);
		return;
	}

	//return; //STOP

	int ModuleID;

	while ((read = getline(&line, &len, fp)) != -1) {
		if(line[0] == '\'')
			continue;

	        printf("\nRetrieved line of length %02zu : ", read);

		char * p = strtok(line,"\t\r\n");
		char * parts[20];
		char i = 0;

		while(p != NULL){
			printf("%s  ",p);
			parts[i++] = p;
			p = strtok(NULL, "\t\r\n");
		}
		printf("\n");

		if(parts[0][0] == 'C'){
			if(strcmp(parts[0],"CU") == 0){ //Create Unit
				//Set Module ID for this file and Create Module
				ModuleID = atoi(parts[1]);
				printf("Module ID: %i\n",ModuleID);
				Create_Unit2(ModuleID,atoi(parts[2]),atoi(parts[3]));

			}else if(strcmp(parts[0],"CB") == 0){ //Create Block
				//Create a Segment with all given data from the file

				struct SegC Adr,NAdr,PAdr;
				Adr = CAdr(ModuleID,atoi(parts[2]),parts[3][0]);
				printf("New block in module %i",ModuleID);
				//Next Block
				if(parts[4][0] == 'C'){
					NAdr = CAdr(ModuleID,0,'C');
				}
				else if(parts[4][0] == 'E'){ //End of line / Empty
					NAdr = EMPTY_BL();
				}
				else{
					if(parts[4][0] == 'X'){
						NAdr = CAdr(ModuleID,atoi(parts[5]),parts[6][0]);
					}else{
						NAdr = CAdr(atoi(parts[4]),atoi(parts[5]),parts[6][0]);
					}
				}

				//Prev Block
				if(parts[7][0] == 'C'){
					PAdr = CAdr(ModuleID,0,'C');
				}
				else if(parts[7][0] == 'E'){
					PAdr = EMPTY_BL();
				}
				else{
					if(parts[7][0] == 'X'){
						printf("Prev block is in same module %i \n",ModuleID);
						PAdr = CAdr(ModuleID,atoi(parts[8]),parts[9][0]);
					}else{
						PAdr = CAdr(atoi(parts[7]),atoi(parts[8]),parts[9][0]);
					}
				}
			  //Create_Segment(IO_Adr        ,Adr,Next,Prev,mspd,           state,dir,            len);
				Create_Segment(atoi(parts[1]),Adr,NAdr,PAdr,atoi(parts[10]),GREEN,atoi(parts[11]),atoi(parts[12]));
				//Set oneway
				if(parts[13][0] == 'Y'){
					Units[ModuleID]->B[Adr.Adr]->oneWay = TRUE;
				}
			}else if(strcmp(parts[0],"CSw") == 0){//Create Switch
				//Create a Switch with all given data from the file

				struct SegC Adr,AAdr,SAdr,DAdr;
				Adr = CAdr(ModuleID,atoi(parts[1]),atoi(parts[12]));

				//Approach Block
				if(parts[3][0] == 'C'){
					AAdr = CAdr(ModuleID,0,'C');
				}
				else if(parts[3][0] == 'E'){
					AAdr = EMPTY_BL();
				}
				else{
					if(parts[3][0] == 'X'){
						AAdr = CAdr(ModuleID,atoi(parts[4]),parts[5][0]);
					}else{
						AAdr = CAdr(atoi(parts[3]),atoi(parts[4]),parts[5][0]);
					}
				}

				//Diverging Block 9+
				if(parts[9][0] == 'C'){
					DAdr = CAdr(ModuleID,0,'C');
				}
				else if(parts[9][0] == 'E'){
					DAdr = EMPTY_BL();
				}
				else{
					if(parts[9][0] == 'X'){
						DAdr = CAdr(ModuleID,atoi(parts[10]),parts[11][0]);
					}else{
						DAdr = CAdr(atoi(parts[9]),atoi(parts[10]),parts[11][0]);
					}
				}

				//Straigth Block 6
				if(parts[6][0] == 'C'){
					SAdr = CAdr(ModuleID,0,'C');
				}
				else if(parts[6][0] == 'E'){
					SAdr = EMPTY_BL();
				}
				else{
					if(parts[6][0] == 'X'){
						SAdr = CAdr(ModuleID,atoi(parts[7]),parts[8][0]);
					}else{
						SAdr = CAdr(atoi(parts[6]),atoi(parts[7]),parts[8][0]);
					}
				}

				int IOAddress[20];
				char * q;
				i = 0;
				q = strtok(parts[12], " ");

				while(q != NULL){
					IOAddress[i++] = atoi(q);
					q = strtok(NULL, " ");
				}

				int StateSpeed[20];
				i = 0;
				q = strtok(parts[13], " ");

				while(q != NULL){
					StateSpeed[i++] = atoi(q);
					q = strtok(NULL, " ");
				}

				Create_Switch(Adr,AAdr,DAdr,SAdr,IOAddress,0);

				//Units[ModuleID]->S[Adr.id]->Detection_Block = atoi(parts[2]);
			}else if(strcmp(parts[0],"CSi") == 0){//Create Signal
				printf("Create Signals - Not Supported");
			}else if(strcmp(parts[0],"CSt") == 0){//Create Station
				printf("Create Station/Stop");

				char name[30];
				strcpy(name,parts[1]);

				char type = atoi(parts[2]);
				char NrBlocks = atoi(parts[3]);

				int Blocks[10];
				char * q;
				i = 0;
				q = strtok(parts[4], " ");

				while(q != NULL){
					Blocks[i++] = atoi(q);
					q = strtok(NULL, " ");
				}

				Create_Station(ModuleID,name,type,NrBlocks,Blocks);
			}
		}
		else if(parts[0][0] == 'S'){
			if(strcmp(parts[0],"Sdet") == 0){ //Switch detection block
				Units[ModuleID]->S[atoi(parts[1])]->Detection_Block = Units[ModuleID]->B[atoi(parts[2])];
			}
		}
 	}

	fclose(fp);
	if (line)
        free(line);
}

void JoinModules(){
	if((_SYS->_STATE & STATE_Modules_Loaded) == 0){
		//No track loaded
		return;
	}
	printf("Ready to join modules\n");

	struct ConnectList * List = (struct ConnectList *)calloc(30,sizeof(struct ConnectList));


	int i = 0;
	int max_j = init_connect_Algor(List);
	int cur_j = max_j;
	int prev_j = max_j;
	while((_SYS->_STATE & STATE_Modules_Coupled) == 0){
		cur_j = connect_Algor(List);
		if(i > 70){
			printf(" (%02i/%02i)\n",cur_j,max_j);
			i = 0;
		}
		if(prev_j == cur_j){
			printf(".");
		}else{
			printf("+");
		}
		i++;
		usleep(200000);
		prev_j = cur_j;

		if(i == 1){
			printf("\n1\n");
			Units[1]->B[0]->blocked = 1;
			Units[8]->B[0]->blocked = 1;

			Units[1]->B[3]->blocked = 0;
			Units[8]->B[4]->blocked = 0;
		}else if(i == 2){
			printf("\n11\n");
			Units[8]->B[3]->blocked = 1;
			Units[4]->B[0]->blocked = 1;

			Units[1]->B[0]->blocked = 0;
			Units[8]->B[0]->blocked = 0;
		}else if(i == 3){
			printf("\n21\n");
			Units[4]->B[11]->blocked = 1;
			Units[2]->B[ 0]->blocked = 1;

			Units[8]->B[3]->blocked = 0;
			Units[4]->B[0]->blocked = 0;
		}else if(i == 4){
			printf("\n31\n");
			Units[2]->B[ 3]->blocked = 1;
			Units[4]->B[27]->blocked = 1;

			Units[4]->B[11]->blocked = 0;
			Units[2]->B[ 0]->blocked = 0;
		}else if(i == 5){
			printf("\n41\n");
			Units[8]->B[ 7]->blocked = 1;
			Units[4]->B[12]->blocked = 1;

			Units[4]->B[27]->blocked = 0;
			Units[2]->B[ 3]->blocked = 0;
		}else if(i == 6){
			printf("\n51\n");
			Units[1]->B[3]->blocked = 1;
			Units[8]->B[4]->blocked = 1;

			Units[8]->B[ 7]->blocked = 0;
			Units[4]->B[12]->blocked = 0;
		}else if(i == 7){
			_SYS->_STATE |= STATE_Modules_Coupled;
			Units[1]->B[3]->blocked = 0;
			Units[8]->B[4]->blocked = 0;
		}
		//IF ALL JOINED
		//BREAK
	}

	int length = List[0].length;
	for(int i = 0;i<length;i++){
		if(List[i].Switch){
			if(List[i].Switch->AppC.type == 'C'){
				List[i].Switch->AppC.type = 0;
			}else if(List[i].Switch->StrC.type == 'C'){
				List[i].Switch->StrC.type = 0;
			}else if(List[i].Switch->DivC.type == 'C'){
				List[i].Switch->DivC.type = 0;
			}
		}else if(List[i].Block){
			if(List[i].Block->NextC.type == 'C'){
				List[i].Block->NextC.type = 0;
			}else if(List[i].Block->PrevC.type == 'C'){
				List[i].Block->PrevC.type = 0;
			}
		}
	}

	free(List);
}
