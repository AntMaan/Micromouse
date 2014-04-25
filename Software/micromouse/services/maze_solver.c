#include "maze_solver.h"
#include <stddef.h>

static cNode_t nodes[xSize][ySize];

int8_t currentPositionX=0,currentPositionY=0;
int8_t nextPositionX=0,nextPositionY=0;
int8_t initialPositionX=0, initialPositionY=0;
int16_t rotation = 0;
int16_t initialRotation=0;
cNodeInfo_t dummyNode={16,255,0,-1,-1};
//Prototypes for private functions
void maze_get_path(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY);
uint8_t maze_resolve_direction(int8_t nextX, int8_t nextY, int8_t currentX, int8_t currentY);
void maze_patch_holes(void);
//Initializer. Must be called to set-up the maze map
void maze_solver_init(void)
{
	uint8_t r;
	uint8_t c;
	for(r=0;r<ySize;r++){
		for(c=0;c<xSize;c++){
			nodes[r][c].NodeInfo.x=r;
			nodes[r][c].NodeInfo.y=c;
			nodes[r][c].NodeInfo.distance=255;
			nodes[r][c].NodeInfo.Flags=15;
			nodes[r][c].previous=&dummyNode;
		}
	}

}

	void maze_init_ff(void)
	{
		uint8_t i1=((xSize+1)/2);
		uint8_t i2=((xSize-1)/2);
		uint8_t j1=((ySize+1)/2);
		uint8_t j2=((ySize-1)/2);
		uint8_t r;
		uint8_t c;
		for(r=0;r<xSize;r++){
			for(c=0;c<ySize;c++){
				nodes[r][c].NodeInfo.distance=0;
				nodes[r][c].NodeInfo.Flags=0;
				if(abs(r-i2)>abs(r-i1))
					nodes[r][c].NodeInfo.distance+=abs(r-i1);
				else
					nodes[r][c].NodeInfo.distance+=abs(r-i2);

				if(abs(c-j2)>abs(c-j1))
					nodes[r][c].NodeInfo.distance+=abs(c-j1);
				else
					nodes[r][c].NodeInfo.distance+=abs(c-j2);
			}
		}
	}

void maze_clear(void)
{
	uint8_t r;
		uint8_t c;
		for(r=0;r<ySize;r++){
			for(c=0;c<xSize;c++){
				nodes[r][c].NodeInfo.distance=255;
				nodes[r][c].NodeInfo.Flags=15;
				nodes[r][c].previous=&dummyNode;
			}
		}
		rotation=initialRotation;
		currentPositionX=initialPositionX;
		currentPositionY=initialPositionY;

}

void maze_set_start_rotation(uint16_t inRotation)
{
	initialRotation=inRotation;
	rotation=inRotation;
}

void maze_set_start_point(uint8_t x, uint8_t y)
{
	initialPositionX=x;
	initialPositionY=y;
	currentPositionX=x;
	currentPositionY=y;
}

	//Flag inputs are raw data. Rotation and position are tracked within the Maze Module.
void maze_update_node(uint8_t Flags)
{
	//Uncomment this line if you don't want to refresh previously visited zones
	//if((nodes[currentPositionX][currentPositionY].NodeInfo.Flags&16)>>4<=0){

		uint8_t flipCount=rotation/90;
		if(flipCount==0)
			nodes[currentPositionX][currentPositionY].NodeInfo.Flags=(Flags&15)+(16);
		else if(flipCount==1)
			nodes[currentPositionX][currentPositionY].NodeInfo.Flags=((Flags&1)<<4-flipCount)|((Flags&15)>>flipCount)+(16);
		else if(flipCount==2)
			nodes[currentPositionX][currentPositionY].NodeInfo.Flags=((Flags&3)<<4-flipCount)|((Flags&15)>>flipCount)+(16);
		else if(flipCount==3)
			nodes[currentPositionX][currentPositionY].NodeInfo.Flags=((Flags&7)<<4-flipCount)|((Flags&15)>>flipCount)+(16);
	//}
}


//Returns 0-3 to indicate the mouse should go 0:Straight,1:Right,2:Backwards,3:Left
//Returns -1 if the mouse returns to the home position
int8_t maze_next_direction_dfs(void)
{
	uint8_t Flags=nodes[currentPositionX][currentPositionY].NodeInfo.Flags;
	int8_t choice=-1;

	//Check all neighbors to find a suitable route.
	if(currentPositionX-1>=0){
		if((Flags&1)==0&&(nodes[currentPositionX-1][currentPositionY].NodeInfo.Flags>>4&1)==0){
			choice=0;
		}
	}
	if(currentPositionY-1>=0){
		if((Flags>>1&1)==0&&(nodes[currentPositionX][currentPositionY-1].NodeInfo.Flags>>4&1)==0){
			choice=1;
		}
	}
	if(currentPositionX+1<xSize){
		if((Flags>>2&1)==0&&(nodes[currentPositionX+1][currentPositionY].NodeInfo.Flags>>4&1)==0){
			choice=2;
		}
	}
	if(currentPositionY+1<ySize){
		if((Flags>>3&1)==0&&(nodes[currentPositionX][currentPositionY+1].NodeInfo.Flags>>4&1)==0){
			choice=3;
		}
	}
	//Rewind
	//This assumes a dead end, must return the direction that was originally traveled. WRONG
	if(choice<0){
		nextPositionX=nodes[currentPositionX][currentPositionY].previous->x;
		nextPositionY=nodes[currentPositionX][currentPositionY].previous->y;
		//We have found the root node. Return -1 to signal the maze is complete.
		if(nextPositionX<0||nextPositionY<0)
			return -1;
	}
	//Resolve choice to nextPosition
	else{
		if(choice==0){
			nextPositionX=currentPositionX-1;
			nextPositionY=currentPositionY;
		}
		else if(choice==1){
			nextPositionX=currentPositionX;
			nextPositionY=currentPositionY-1;
		}
		else if(choice==2){
			nextPositionX=currentPositionX+1;
			nextPositionY=currentPositionY;
		}
		else if(choice==3){
			nextPositionX=currentPositionX;
			nextPositionY=currentPositionY+1;
		}
		nodes[nextPositionX][nextPositionY].previous=&nodes[currentPositionX][currentPositionY].NodeInfo;
		//Link previous node to next node.

	}

	return maze_resolve_direction(nextPositionX,nextPositionY, currentPositionX, currentPositionY);
}



	static cNodeInfo_t* ffStack[xSize*ySize];
	static int8_t neighborDist[4]={-1,-1,-1,-1};
	static int8_t weightedNeighborDist[4]={-1,-1,-1,-1};
	int8_t index=0;
	int8_t maze_next_direction_ff(void)
	{
		uint8_t Flags=nodes[currentPositionX][currentPositionY].NodeInfo.Flags;
		uint8_t choice=255;
		int8_t X=currentPositionX;
		int8_t Y=currentPositionY;
		neighborDist[0]=-1;
		neighborDist[1]=-1;
		neighborDist[2]=-1;
		neighborDist[3]=-1;
		weightedNeighborDist[0]=-1;
		weightedNeighborDist[1]=-1;
		weightedNeighborDist[2]=-1;
		weightedNeighborDist[3]=-1;
		//Push current Cell onto Stack
		ffStack[index]=&nodes[X][Y].NodeInfo;
		//Check if the mouse is on the final position
		if(nodes[X][Y].NodeInfo.distance==0)
			return -1;

		//Check all neighbors
		if(X-1>=0){
			if((Flags&1)==0){
				neighborDist[0]=nodes[X-1][Y].NodeInfo.distance;
				weightedNeighborDist[0]=nodes[X-1][Y].NodeInfo.distance+nodes[X-1][Y].NodeInfo.VisitedCount;
			}
		}
		if(Y-1>=0){
			if((Flags>>1&1)==0){
				neighborDist[1]=nodes[X][Y-1].NodeInfo.distance;
				weightedNeighborDist[1]=nodes[X][Y-1].NodeInfo.distance+nodes[X][Y-1].NodeInfo.VisitedCount;
			}
		}
		if(X+1<xSize){
			if((Flags>>2&1)==0){
				neighborDist[2]=nodes[X+1][Y].NodeInfo.distance;
				weightedNeighborDist[2]=nodes[X+1][Y].NodeInfo.distance+nodes[X+1][Y].NodeInfo.VisitedCount;
			}
		}
		if(Y+1<ySize){
			if((Flags>>3&1)==0){
				neighborDist[3]=nodes[X][Y+1].NodeInfo.distance;
				weightedNeighborDist[3]=nodes[X][Y+1].NodeInfo.distance+nodes[X][Y+1].NodeInfo.VisitedCount;

			}
		}
		uint8_t dist=255;
		//Decide which way we will go
		uint8_t i=0;
		for(i=0;i<4;i++)
		{
			if(weightedNeighborDist[i]<dist&&weightedNeighborDist[i]>-1){
							dist=weightedNeighborDist[i];
							choice=i;
						}
		}
		dist=neighborDist[choice];
		int8_t safety = 100;
		//If there is an inconsistency, backtrack and fix distances
		if(nodes[currentPositionX][currentPositionY].NodeInfo.distance<=neighborDist[choice])
		{
			while(index>=0&&safety>=0)
			{
				safety--;
				uint8_t minDist=255;
				X=ffStack[index]->x;
				Y=ffStack[index]->y;
				Flags=nodes[X][Y].NodeInfo.Flags;
				neighborDist[0]=-1;
				neighborDist[1]=-1;
				neighborDist[2]=-1;
				neighborDist[3]=-1;

				//Check all neighbors
				if(X-1>=0){
					if((Flags&1)==0){
						neighborDist[0]=nodes[X-1][Y].NodeInfo.distance;
					}
				}
				if(Y-1>=0){
					if((Flags>>1&1)==0){
						neighborDist[1]=nodes[X][Y-1].NodeInfo.distance;
					}
				}
				if(X+1<xSize){
					if((Flags>>2&1)==0){
						neighborDist[2]=nodes[X+1][Y].NodeInfo.distance;
					}
				}
				if(Y+1<ySize){
					if((Flags>>3&1)==0){
						neighborDist[3]=nodes[X][Y+1].NodeInfo.distance;
					}
				}

				uint8_t i;
				//Get minimum distance
				for(i=0;i<4;i++)
				{
					if(neighborDist[i]<minDist&&neighborDist[i]>-1){
						minDist=neighborDist[i];
					}
				}

				//if equal to one greater than min, make it equal to then continue
				if(ffStack[index]->distance==minDist+1){
					index--;
				   continue;
				}
				else
				{
					X=ffStack[index]->x;
					Y=ffStack[index]->y;
					ffStack[index]->distance=minDist+1;
					nodes[X][Y].NodeInfo.distance=minDist+1;
					index--;
					if(X-1>=0){
						if((Flags>>0&1)==0){

							ffStack[++index]=&nodes[X-1][Y].NodeInfo;
						}
					}
					if(Y-1>=0){
						if((Flags>>1&1)==0){
							ffStack[++index]=&nodes[X][Y-1].NodeInfo;
						}
					}
					if(X+1<xSize){
						if((Flags>>2&1)==0){
							ffStack[++index]=&nodes[X+1][Y].NodeInfo;
						}
					}
					if(Y+1<ySize){
						if((Flags>>3&1)==0){
							ffStack[++index]=&nodes[X][Y+1].NodeInfo;
						}
					}
				}
			}
			//index++;
		}

		if(choice==0){
			nextPositionX=currentPositionX-1;
			nextPositionY=currentPositionY;
		}
		else if(choice==1){
			nextPositionX=currentPositionX;
			nextPositionY=currentPositionY-1;
		}
		else if(choice==2){
			nextPositionX=currentPositionX+1;
			nextPositionY=currentPositionY;
		}
		else if(choice==3){
			nextPositionX=currentPositionX;
			nextPositionY=currentPositionY+1;
		}

		if((nodes[nextPositionX][nextPositionY].NodeInfo.Flags>>4&1)==0){
			nodes[nextPositionX][nextPositionY].previous=&nodes[currentPositionX][currentPositionY].NodeInfo;
		}
		nodes[currentPositionX][currentPositionY].NodeInfo.VisitedCount+=1;
		index++;
		return maze_resolve_direction(nextPositionX,nextPositionY, currentPositionX, currentPositionY);
	}

int8_t shortestPath[xSize*ySize];
int8_t * maze_dijkstras_algorithm(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY){
	uint8_t r;
	uint8_t c;
	maze_patch_holes();
	//Start all nodes at default values and remove visited flags
	for(r=0;r<ySize;r++){
		for(c=0;c<xSize;c++){
			nodes[r][c].NodeInfo.distance=xSize*ySize-1;
			nodes[r][c].NodeInfo.Flags-=16;
			nodes[r][c].previous=&dummyNode;
			shortestPath[r*xSize+c]=-1;
		}
	}

	//Initialize first node to 0 distance
	nodes[startX][startY].NodeInfo.distance=0;

	//Start at max number of nodes. Simply here for safety purposes
	int16_t nodeSize=xSize*ySize-1;



	while(nodeSize>=0){
		nodeSize--;

		cNodeInfo_t min;
		min.x=0;
		min.y=0;
		min.distance=xSize*ySize-1;

		uint8_t r;
		uint8_t c;
		for(r=0;r<ySize;r++){
			for(c=0;c<xSize;c++){
				if((nodes[c][r].NodeInfo.distance<min.distance)&&(nodes[c][r].NodeInfo.Flags>>4&1)==0){
					min=nodes[c][r].NodeInfo;
				}
			}
		}

		//Set node as visited
		nodes[min.x][min.y].NodeInfo.Flags+=16;

		if(min.x==endX&&min.y==endY){
			maze_get_path(startX,startY,endX,endY);
			return shortestPath;
		}
		else if(min.distance==xSize*ySize)
			return shortestPath;

		uint8_t alt=0;

		if(((min.Flags>>3&1)==0)&&min.y+1<15){
			alt = min.distance + 1;
			if(alt < nodes[min.x][min.y+1].NodeInfo.distance)
			{
				nodes[min.x][min.y+1].NodeInfo.distance=alt;
				nodes[min.x][min.y+1].previous=&nodes[min.x][min.y].NodeInfo;
			}
		}

		if(((min.Flags>>2&1)==0)&&min.x+1<15){
			alt = min.distance + 1;
			if(alt < nodes[min.x+1][min.y].NodeInfo.distance)
			{
				nodes[min.x+1][min.y].NodeInfo.distance=alt;
				nodes[min.x+1][min.y].previous=&nodes[min.x][min.y].NodeInfo;
			}
		}

		if(((min.Flags>>1&1)==0)&&min.y-1>=0){
			alt = min.distance + 1;
			if(alt < nodes[min.x][min.y-1].NodeInfo.distance)
			{
				nodes[min.x][min.y-1].NodeInfo.distance=alt;
				nodes[min.x][min.y-1].previous=&nodes[min.x][min.y].NodeInfo;
			}
		}

		if(((min.Flags&1)==0)&&min.x-1>=0){
			alt = min.distance + 1;
			if(alt < nodes[min.x-1][min.y].NodeInfo.distance)
			{
				nodes[min.x-1][min.y].NodeInfo.distance=alt;
				nodes[min.x-1][min.y].previous=&nodes[min.x][min.y].NodeInfo;
			}
		}

	}
	return shortestPath;
}

void maze_patch_holes(void)
{
	uint8_t r;
	uint8_t c;
	for(r=0;r<ySize;r++){
				for(c=0;c<xSize;c++){
					if((nodes[c][r].NodeInfo.Flags>>4&1)==0){
						nodes[c][r].NodeInfo.Flags=15;
					}
				}
			}
}

uint16_t maze_get_rotation(void)
{
	return rotation;
}

//This function returns the current x and y position of the micro-mouse.
void maze_get_position(uint8_t *X, uint8_t *Y)
{
	*X=currentPositionX;
	*Y=currentPositionY;
}

void maze_get_path(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY)
{
	uint8_t index=nodes[endX][endY].NodeInfo.distance-1;
	int8_t dir=0;
	int8_t tempX=0;
	shortestPath[index]=-1;
	currentPositionX=endX;
	currentPositionY=endY;



	int8_t Xdir=nodes[currentPositionX][currentPositionY].previous->x-currentPositionX;
	int8_t Ydir=nodes[currentPositionX][currentPositionY].previous->y-currentPositionY;

	tempX=currentPositionX;

	currentPositionX=nodes[currentPositionX][currentPositionY].previous->x;
	currentPositionY=nodes[tempX][currentPositionY].previous->y;

	if(Ydir!=0)
		if(Ydir>0)
			dir=2;
	else
		dir=0;
	else if(Xdir!=0)
		if(Xdir>0)
			dir=3;
	else
		dir=1;

	//Resolve initial rotation
	rotation=90*dir;

	do{
		dir=maze_resolve_direction(nodes[currentPositionX][currentPositionY].previous->x,nodes[currentPositionX][currentPositionY].previous->y,currentPositionX,currentPositionY);
		rotation-=180;
		if(rotation<0)
			rotation+=360;

		dir-=2;
		if(dir<0)
			dir+=4;

		if(dir==1)
			dir=3;
		else if(dir==3)
			dir=1;


		shortestPath[index]=dir;
		index--;

	}while(nodes[currentPositionX][currentPositionY].previous->x>-1&&nodes[currentPositionX][currentPositionY].previous->y>-1);

	dir=(rotation-initialRotation)/90;
	if(dir<0)
		dir+=4;
	shortestPath[0]=dir;
}

//Update Rotation x is row, y is column
uint8_t maze_resolve_direction(int8_t nextX, int8_t nextY, int8_t currentX, int8_t currentY)
{
	int8_t Xdir=nextX-currentX;
	int8_t Ydir=nextY-currentY;
	int8_t dir=0;

	if(Ydir!=0)
		if(Ydir>0)
			dir=0;
		else
			dir=2;
	else if(Xdir!=0)
		if(Xdir>0)
			dir=1;
		else
			dir=3;

	currentPositionX=nextX;
	currentPositionY=nextY;
	dir=dir-rotation/90;
	if(dir<0)
		dir=dir+4;

	rotation+=dir*90;
	if (rotation>=360)
		rotation-=360;

	return dir;
}
