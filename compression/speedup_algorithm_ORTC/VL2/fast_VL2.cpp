#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include<time.h>
#include <string.h>
using namespace std;
#include "VL2.h"


int createSpanningTree(int (*ST)[TotalTNum]);
int SwitchOptimalEncoding(int *FinalID, int (*ST)[TotalTNum]);
int currentMS_init(int *currentMS, int *LocateTable, int (*ST)[TotalTNum]);
int LocateTable_init(int *LocateTable, int *FinalID, int (*ST)[TotalTNum]);
int IDencoding_ORTC(int (*ST)[TotalTNum], ofstream &output);


int main(){
	int (*ST)[TotalTNum]=new int[Smax][TotalTNum];
	clock_t start, finish;   
	double  duration; 
	ofstream output("Output_VL2.txt",ios::app);
	output<<setiosflags(ios::fixed)<<setprecision(6);

	output<<"DA= "<<DA<<" DI= "<<DI<<endl; 

	memset(ST,0,sizeof(int)*Smax*TotalTNum);
	srand((unsigned)time(0));
             
	start=clock();   
	createSpanningTree(ST);
	finish=clock();   
	duration=(double)(finish-start)/CLOCKS_PER_SEC; 
	//cout<<"The running time of creating spanning trees is: "<<duration<<endl; 
	output<<"The running time of creating spanning trees is: "<<duration<<endl; 

/* for(int i=0; i<Smax; i++){
	  cout<<"Switch number "<<i<<": ";
      for(int j=0; j<TotalTNum; j++)
          cout<<ST[i][j]<<" ";
	  cout<<endl;
  }*/   
	start=clock();   
	IDencoding_ORTC(ST,output);
	finish=clock();   
	duration=(double)(finish-start)/CLOCKS_PER_SEC; 
	output<<"The running time of compute maxORT is: "<<duration<<endl; 

  delete[] ST;
  output.close();

  return 0;
}


int createSpanningTree(int (*ST)[TotalTNum]){

	for(int DesTorSwitch=0; DesTorSwitch<TorSNum; DesTorSwitch++)
		for(int IntSwitch=0; IntSwitch<IntSNum; IntSwitch++){
			int TNum=DesTorSwitch*SingleTNum+IntSwitch*2;
			for(int num=0; num<2; num++){
				int CurrentTnum=TNum+num;
				int DesAggrSwitch=0;
				for(int AggrSwitch=0; AggrSwitch<AggrSNum; AggrSwitch+=2){
					for(int AggrPort=0; AggrPort<DA/2; AggrPort++){
						int CurrentTor=AggrSwitch/2*DA/2+AggrPort;
						if(CurrentTor!=DesTorSwitch){
							if( num==0 )
								//ST[CurrentTor][CurrentTnum]=ST[CurrentTor][CurrentTnum]|const_1;
								ST[CurrentTor][CurrentTnum]=1;
							else
								//ST[CurrentTor][CurrentTnum]=ST[CurrentTor][CurrentTnum]|(const_1<<1);
								ST[CurrentTor][CurrentTnum]=2;
						}else{
							//ST[TorSNum+AggrSwitch+num][CurrentTnum]=ST[TorSNum+AggrSwitch+num][CurrentTnum]|(const_1<<AggrPort);
							ST[TorSNum+AggrSwitch+num][CurrentTnum]=AggrPort+1;
							DesAggrSwitch=TorSNum+AggrSwitch+num;
						}
					}     
				}
				//ST[IntSwitch+TorSNum+AggrSNum][CurrentTnum]=ST[IntSwitch+TorSNum+AggrSNum][CurrentTnum]|( const_1<<(DesAggrSwitch-TorSNum) );
				ST[IntSwitch+TorSNum+AggrSNum][CurrentTnum]=DesAggrSwitch-TorSNum+1;
				for(int i=TorSNum+num; i<TorSNum+AggrSNum; i=i+2)
					if(i!=DesAggrSwitch)
						//ST[i][CurrentTnum]=ST[i][CurrentTnum]|( const_1<<(IntSwitch+DA/2) );
						ST[i][CurrentTnum]=IntSwitch+DA/2+1;
			}
		}



		return 0;
}

int SwitchOptimalEncoding(int s, int *FinalID, int (*ST)[TotalTNum]){
	int CurrentID;
	int *flag=new int[TotalTNum];
	memset(flag,0, sizeof(int)*TotalTNum);
	CurrentID=0;
	for(int i=0; i<TotalTNum; i++)
		if(flag[i]==0) {
			flag[i]=1;
			FinalID[i]=CurrentID++;
			for(int j=i+1; j<TotalTNum; j++)        
				if(ST[s][j]==ST[s][i]){
					flag[j]=1;
					FinalID[j]=CurrentID++;
				}
		} 
	delete[] flag;
	return 0;

}


int currentMS_init(int *currentMS, int *LocateTable, int (*ST)[TotalTNum]){
	for(int i=0; i<Smax; i++){
		int j, pre;
		pre=0;
		currentMS[i]=1;
		for(j=0; j<TotalTNum;j++)
			if(ST[i][ LocateTable[j] ]!=0){
				pre=ST[i][ LocateTable[j] ];
				break;
			}
		if(pre!=0)
			for( j++; j<TotalTNum; j++)
				if(ST[i][ LocateTable[j] ] != 0 && pre!=ST[i][ LocateTable[j] ]){
					currentMS[i]++;
					pre=ST[i][ LocateTable[j] ];
				}
	}
	return 0;

}
int LocateTable_init(int *LocateTable, int *FinalID){

	for(int tree=0; tree<TotalTNum; tree++)
		LocateTable[FinalID[tree]]=tree;
return 0;
}


int computeMinMax(int KeySwitch, int *FinalID,
					  int *LocateTable, int *currentMS, int (*ST)[TotalTNum]){
			int answer=0;

			SwitchOptimalEncoding(KeySwitch, FinalID, ST);
			LocateTable_init(LocateTable, FinalID);
		    currentMS_init(currentMS, LocateTable, ST);
			for(int i=0; i<Smax; i++)
				if(currentMS[i]>answer){
					answer=currentMS[i];
				}
			//cout<<"after conflict correcting_3: "<<answer<<endl;
			/*			int index;
			for(int i=0; i<TotalTNum; i++)
				cout<<ST[index][LocateTable[i]]<<" ";
			cout<<endl;*/ 

			return answer;


}


class ORTC_node{
public:
	unsigned int portmap[MapLen];
	unsigned int routemap[MapLen];
	int prefix_enable;
};

int local_OptIP(int s, int * flag, int *IP, int (*ST)[TotalTNum], int maxIP){
	unsigned int blocksz[Pmax+1]={0};
	int porder[Pmax+1]={0};

	memset(flag, 0, sizeof(int)*TotalTNum);

	for(int i=0; i<TotalTNum; i++) //compute block size;
		if(flag[i]==0) {
			flag[i]=1;
			blocksz[ST[s][i]]=1;
			for(int j=i+1; j<TotalTNum; j++)        
				if(ST[s][j]==ST[s][i]){
					flag[j]=1;
					blocksz[ST[s][i]]++;
				}
		}

		//  for(int i=0; i<=Pmax; i++)
		//    cout<<" blocksz["<<i<<"]= "<<blocksz[i];
		//  cout<<endl;

		int sorted[Pmax+1];
		memset(sorted, 0, sizeof(int)*(Pmax+1) );
		for(int round=0; round<=Pmax; round++){
			unsigned int maxbsz=0;
			int maxp;
			for(int p=0; p<=Pmax; p++) 
				if(sorted[p]==0 && blocksz[p]>maxbsz){
					maxbsz=blocksz[p];
					maxp=p;
				}
				if(maxbsz>0){
					porder[round]=maxp;
					sorted[maxp]=1;
				} else
					porder[round]=-1;
		}
		//for(int i=0; i<=Pmax; i++)
		//  cout<<" porder["<<i<<"]= "<<porder[i];
		//cout<<endl;

		int b_startIP=0;
		int curIP=b_startIP;
		for(int round=0; round<=Pmax; round++)
			if( porder[round]>=0 ){
				int curport=porder[round];
				//	cout<<" curport "<<curport<<endl;
				for(int ps=0; ps<TotalTNum; ps++)
					if(ST[s][ps]==curport)
						IP[ps]=curIP++;
				if(curIP>maxIP)
					cout<<"error1"<<endl;
				for(int i=0; i<IPv4len; i++)
					if(IPsegsz[i]>=blocksz[curport]){
						b_startIP+=IPsegsz[i];
						curIP=b_startIP;
						break;
					}
			}
			return 0;
}

int compute_ORT(int s, int *IP, int ORTC_nnum, int ORTC_leafnnum,
				ORTC_node *ORTC_tree, int (*ST)[TotalTNum]){
					int leaf_startn=ORTC_nnum-ORTC_leafnnum;
					for(int node=0; node<ORTC_nnum; node++){
						memset(ORTC_tree[node].portmap, 0, sizeof(int)*(MapLen) );
						memset(ORTC_tree[node].routemap, 0, sizeof(int)*(MapLen) );
					}
					for(int node=leaf_startn; node<ORTC_nnum; node++)
						ORTC_tree[node].portmap[0]=1;
					int ip=0;
					int outport=0;
					for(int ps=0; ps<TotalTNum; ps++){
						ip=IP[ps];
						outport=ST[s][ps];
						ORTC_tree[leaf_startn+ip].portmap[0]=0;
						if( ip<0 || (leaf_startn+ip)>=ORTC_nnum)
							cout<<"error2"<<endl;
						for(int map=0; map<MapLen; map++)
							if(Intbitlen*(map+1)>outport){
								ORTC_tree[leaf_startn+ip].portmap[map]= 1 << (outport-map*Intbitlen);
								break;
							}
					}

					for(int node=leaf_startn-1; node>=0; node--){
						int lch=2*node+1;
						int rch=2*node+2;
						if( (rch)>ORTC_nnum)
							cout<<"error3"<<endl;
						int intersec=0;
						for(int map=0; map<MapLen; map++)
							if( (ORTC_tree[lch].portmap[map] & ORTC_tree[rch].portmap[map])!=0 ){
								intersec=1;
								break;
							}
							if(intersec==0){
								for(int map=0; map<MapLen; map++)
									ORTC_tree[node].portmap[map]= ORTC_tree[lch].portmap[map] | ORTC_tree[rch].portmap[map];
							}else{
								for(int map=0; map<MapLen; map++)
									ORTC_tree[node].portmap[map]= ORTC_tree[lch].portmap[map] & ORTC_tree[rch].portmap[map];
							}
					}

					ORTC_tree[0].prefix_enable=0;
					for(int map=0; map<MapLen; map++)
						if(ORTC_tree[0].portmap[map]!=0){
							for(int bit=0; bit<Intbitlen; bit++)
								if( ( ORTC_tree[0].portmap[map] & (unsigned int)(1<<bit) )!=0 ){
									ORTC_tree[0].routemap[map]=(unsigned int)(1<<bit);
									if(bit!=0 || map!=0)
										ORTC_tree[0].prefix_enable=1;
									break;
								}
								break;
						}
						for(int node=1; node<ORTC_nnum; node++){
							int parent=(node-1)/2;
							int hasdupp=0;
							for(int map=0; map<MapLen; map++)
								if( (ORTC_tree[node].portmap[map] & ORTC_tree[parent].routemap[map])!=0 ){
									hasdupp=1;
									break;
								}
								if(hasdupp==1){
									ORTC_tree[node].prefix_enable=0;
									for(int map=0; map<MapLen; map++)
										ORTC_tree[node].routemap[map]=ORTC_tree[parent].routemap[map];
								}else{
									for(int map=0; map<MapLen; map++)
										if(ORTC_tree[node].portmap[map]!=0){
											for(int bit=0; bit<Intbitlen; bit++)
												if( ( ORTC_tree[node].portmap[map] & (unsigned int)(1<<bit) )!=0 ){
													ORTC_tree[node].routemap[map]=(unsigned int)(1<<bit);
													if(bit!=0 || map!=0)
														ORTC_tree[node].prefix_enable=1;
													else  ORTC_tree[node].prefix_enable=0;
													break;
												}
												break;
										}

										for(int map=0; map<MapLen; map++)
											ORTC_tree[node].routemap[map]=ORTC_tree[node].routemap[map] | ORTC_tree[parent].routemap[map];
								}
						}

						int prefixnum=0;
						for(int node=0; node<ORTC_nnum; node++)
							if(ORTC_tree[node].prefix_enable==1)
								prefixnum++;

						return prefixnum;
}

int CCencoding_IP(int *flag, int *IP, int ORTC_nnum, int ORTC_leafnnum, 
				  ORTC_node *ORTC_tree, int (*ST)[TotalTNum]){
					  //int maxip=ORTC_leafnnum;
					  //int (*er_ST)[TotalTNum]=new int[EclassNum][TotalTNum]; //equivalent-reduction ST;
					  int (*er_IP)[TotalTNum]=new int[EclassNum][TotalTNum]; //equivalent-reduction IP;
					  //int er_IP[EclassNum][TotalTNum]; //equivalent-reduction IP;
					  int min_switch=-1;
					  int min_value=MaxInt;
					  int (*ipindex)[2*TotalTNum]=new int[EclassNum][2*TotalTNum];
					  //int ipindex[EclassNum][2*TotalTNum];
					  int *initial_ips=new int[EclassNum];
					  //int initial_ips[EclassNum];

					  for(int i=0; i<EclassNum; i++){
						  local_OptIP(EclassSID[i], flag, er_IP[i], ST, ORTC_leafnnum);  
					  }


					  memset(ipindex, -1, sizeof(int)*(EclassNum*2*TotalTNum));
					  for(int sid=0; sid<EclassNum; sid++){
						  for(int i=0; i<TotalTNum; i++)
							  ipindex[sid][er_IP[sid][i]]=i;
					  }

					  int considered=0;
					  for(int tree=0; tree<TotalTNum; tree++){
						  min_value=MaxInt;
						  min_switch=-1;
						  for(int base_s=0; base_s<EclassNum; base_s++) {  //enumerate base switch
							  int base_ip=er_IP[base_s][tree];
							  considered=0;
							  for(int c_s=0; c_s<base_s; c_s++){ //check whether base_ip has been considered;
								  int c_ip=er_IP[c_s][tree];
								  if(c_ip==base_ip){
									  considered=1;
									  break;
								  }
							  }
							  if(considered==1)
								  continue;
							  int mrts=compute_ORT(EclassSID[base_s], er_IP[base_s], ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);  //initialize maximum routing table size
							  int temp_rts=0;  //routing table size
							  for(int c_s=0; c_s<EclassNum; c_s++)   //exchange ip
								  if(c_s!=base_s){
									  int c_ip=er_IP[c_s][tree];
									  int exip_index=ipindex[c_s][base_ip]; //get the index of the ip to exchange with c_ip;
									  initial_ips[c_s]=c_ip;
									  er_IP[c_s][tree]=base_ip;
									  if(exip_index!=-1){
										  er_IP[c_s][exip_index]=c_ip;
									  }

									  temp_rts=compute_ORT(EclassSID[c_s], er_IP[c_s], ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
									  if(temp_rts>mrts)
										  mrts=temp_rts;			
								  }
								  if(mrts<min_value){
									  min_value=mrts;
									  min_switch=base_s;
								  }
								  for(int c_s=0; c_s<EclassNum; c_s++)    //recovery
									  if(c_s!=base_s){
										  int exip_index=ipindex[c_s][base_ip];
										  er_IP[c_s][tree]=initial_ips[c_s];
										  if(exip_index!=-1){
											  er_IP[c_s][exip_index]=base_ip;
										  }
									  }
						  }
						  for(int c_s=0; c_s<EclassNum; c_s++)    //exchange ip
							  if(c_s!=min_switch){
								  int min_ip=er_IP[min_switch][tree];
								  int c_ip=er_IP[c_s][tree];
								  int exip_index=ipindex[c_s][min_ip]; //get the index of the ip to exchange with c_ip;
								  er_IP[c_s][tree]=min_ip;
								  ipindex[c_s][c_ip]=-1;
								  ipindex[c_s][min_ip]=tree;
								  if(exip_index!=-1){
									  er_IP[c_s][exip_index]=c_ip;
									  ipindex[c_s][c_ip]=exip_index;
								  }
							  }
					  }
					  for(int i=0; i<TotalTNum; i++)
						  for(int j=1; j<EclassNum; j++)
							  if(er_IP[0][i]!=er_IP[j][i]){
								  cout<<"ip inconsistent!";
								  break;
							  }
							  int *ipcount=new int[TotalTNum*2];
							  for(int sid=0; sid<EclassNum; sid++){
								  memset(ipcount, 0, sizeof(int)*(TotalTNum*2));
								  for(int i=0; i<TotalTNum; i++){
									  ipcount[er_IP[sid][i]]++;
									  if(ipcount[er_IP[sid][i]]>1){
										  cout<<"ip duplication";
										  break;
									  }
								  }

							  }

							  for(int i=0; i<TotalTNum; i++)
								  IP[i]=er_IP[0][i];
							  return 0;
}


int get_maxORT(int KeySwitch, int * flag, int *IP, int ORTC_nnum, int ORTC_leafnnum, 
			   ORTC_node *ORTC_tree, int (*ST)[TotalTNum]){
				   int answer=0;
				   if(KeySwitch==-1){
					   CCencoding_IP(flag, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
				   }else{
				  local_OptIP(KeySwitch, flag, IP, ST, ORTC_leafnnum);
				  // SwitchOptimalEncoding(KeySwitch,  IP, ST);

				 /*  int *isused=new int[TotalTNum];
				   //int isused[TotalTNum];
				   memset(isused, 0, sizeof(int)*(TotalTNum) );
				   for(int i=0; i<TotalTNum; i++) {
					   int ip=(int)( ((double)rand()/(double)RAND_MAX)*(double)TotalTNum);
					   if(ip>=TotalTNum)
						   ip=TotalTNum-1;
					   while(isused[ip]==1)
						   ip=(ip+1)%TotalTNum;
					   IP[i]=ip;
					   isused[ip]=1;
				   } */
				   }

				   for(int i=0; i<Smax; i++){
					   int curORTsz=compute_ORT(i, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
					   if(curORTsz>answer){
						   answer=curORTsz;
					   }
				   }
				   return answer;
}

int IDencoding_ORTC(int (*ST)[TotalTNum], ofstream &output){
	int *IP=new int[TotalTNum];
	int ORTC_nnum;
	int ORTC_leafnnum;
	ORTC_node *ORTC_tree;
	int KeySwitch;
	int FinalAnswer=MaxInt;
	int temp=1;
	int *flag=new int[TotalTNum];

	temp=1;
	for(int i=0; i<IPv4len; i++){
		IPsegsz[i]=temp;
		temp=temp<<1;
	}
	for(int i=0; i<IPv4len; i++)
		if(IPsegsz[i]>=2*TotalTNum){
			ORTC_leafnnum=IPsegsz[i];
			ORTC_nnum=2*ORTC_leafnnum-1;
			break;
		}
	ORTC_tree=new ORTC_node[ORTC_nnum];

	if(CCencoding_enable==1){
		KeySwitch=-1;
		temp=get_maxORT(KeySwitch, flag, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
		if(temp<FinalAnswer)
			FinalAnswer=temp;
	}

	KeySwitch=0;
	temp=get_maxORT(KeySwitch, flag, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
	if(temp<FinalAnswer)
		FinalAnswer=temp;

	KeySwitch=TorSNum;
	temp=get_maxORT(KeySwitch, flag, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
	if(temp<FinalAnswer)
		FinalAnswer=temp;

	KeySwitch=TorSNum+AggrSNum;
	temp=get_maxORT(KeySwitch, flag, IP, ORTC_nnum, ORTC_leafnnum, ORTC_tree, ST);
	if(temp<FinalAnswer)
		FinalAnswer=temp;

	output<<"the total number of paths is: "<<(TotalTNum*(TorSNum-1))<<endl;
	output<<"the total number of path sets is: "<<TotalTNum<<endl;
	output<<"the routing table size is: "<<FinalAnswer<<endl;

	delete[] IP;
	delete[] flag;
	delete[] ORTC_tree;
	return 0;

}
