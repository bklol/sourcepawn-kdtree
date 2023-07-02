/*
	simple kdtree in sp by bklol
	https://github.com/bklol/sourcepawn-kdtree
*/

#pragma dynamic 81920 //40kb is good :)
#define INVALID_NODE -1

#define MAX_SEARCH_NODE 145 
/*
	搜索精度 越大越准确，-1为不受限制， 145 测试于20w节点样本中，精度偏差为+-10tick左右 单人sv为0.002 +— 2 可以被接收
	请注意，当玩家越接近节点坐标，计算所需节点就越少，这和精度没有关系
	暴力搜索代表的点不一定比kdtree搜索的点准确（时空意义上），你所做的一切只是为了获取最近的点，请注意
*/

ArrayList KDtreeData;

enum struct KDNode 
{
	float coord[3];
	int leftIndex;
	int rightIndex;
	int tick;
	void insert(float point[3], int tick)
	{
		this.leftIndex = INVALID_NODE;
		this.rightIndex = INVALID_NODE;
		this.coord[0] = point[0];
		this.coord[1] = point[1];
		this.coord[2] = point[2];
		this.tick = tick;
	}
}

int g_kdTreeRoot = INVALID_NODE;
int g_nextNode = 0;

stock void Init_KDtree()
{
	g_kdTreeRoot = INVALID_NODE;
	g_nextNode = 0;
	if(KDtreeData != null)
		KDtreeData.Clear();
	KDtreeData = new ArrayList(sizeof(KDNode));
}

stock void KDTree_InsertNode(float point[3], int tick)
{
	KDNode newNode;
	newNode.insert(point, tick);
	KDtreeData.PushArray(newNode);
	if (g_kdTreeRoot == INVALID_NODE) 
	{
		g_kdTreeRoot = g_nextNode;
	}
	else
	{
		KDNode parentNode;
		int currentNode = g_kdTreeRoot;
		while(currentNode != INVALID_NODE)
		{

			KDtreeData.GetArray(currentNode, parentNode);
			int childNode;
			if (newNode.coord[currentNode % 3] < parentNode.coord[currentNode % 3]) 
			{
				childNode = parentNode.leftIndex;
				if (childNode == INVALID_NODE) 
				{
					parentNode.leftIndex = g_nextNode;
					KDtreeData.SetArray(currentNode, parentNode);
					break;
				}
			} 
			else 
			{
				childNode = parentNode.rightIndex;
				if (childNode == INVALID_NODE) 
				{
					parentNode.rightIndex = g_nextNode;
					KDtreeData.SetArray(currentNode, parentNode);
					break;
				}
			}
			currentNode = childNode;
		}
	}
	g_nextNode++;
}


stock int KDTree_NearestPosSearch(float target[3])
{
	int bestNode = -1;
	float bestDistance = view_as<float>(0x7f7fffff);
	NearestPosSearchRecursive(g_kdTreeRoot, target, 0, bestDistance, bestNode);
	return bestNode;
}

stock void NearestPosSearchRecursive(int currentNode, float target[3], int SearchDepth, float& bestDistance, int& bestNode) 
{
	if(currentNode == INVALID_NODE ||(SearchDepth > MAX_SEARCH_NODE && MAX_SEARCH_NODE != -1))
		return;
	int axis = SearchDepth % 3;
	KDNode SearchNode;
	KDtreeData.GetArray(currentNode, SearchNode);
	float distance = get_distance(SearchNode.coord, target);
	if (distance < bestDistance) 
	{
		bestDistance = distance;
		bestNode = currentNode;
	}
	
	if(target[axis] < SearchNode.coord[axis])
	{
		NearestPosSearchRecursive(SearchNode.leftIndex, target, SearchDepth + 1, bestDistance, bestNode);
		if(target[axis] + bestDistance >= SearchNode.coord[axis])
		{
			NearestPosSearchRecursive(SearchNode.rightIndex, target, SearchDepth + 1,bestDistance, bestNode);
		}
	}
	else
	{
		NearestPosSearchRecursive(SearchNode.rightIndex, target, SearchDepth + 1, bestDistance, bestNode);
		if(target[axis] - bestDistance <= SearchNode.coord[axis])
		{
			NearestPosSearchRecursive(SearchNode.leftIndex, target, SearchDepth + 1, bestDistance, bestNode);
		}
	}
}

float get_distance(float point_a[3], float point_b[3])
{
	/* 失去精度 不推荐
	return SquareRoot(
		Pow((point_a[0] - point_b[0]), 2.0)+
		Pow((point_a[1] - point_b[1]), 2.0)+
		Pow((point_a[2] - point_b[2]), 2.0));
	*/
	return GetVectorDistance(point_a, point_b, true);
}

stock int GetClosestReplayFrame(int client)//搜索玩家最近坐标
{
	float fClientPos[3];
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", fClientPos);
	return KDTree_NearestPosSearch(fClientPos);
	/*
	//暴力查询结果 用于debug
	float fMinDist = view_as<float>(0x7f7fffff);
	int iClosestFrame = -1;
	for(int frame = 0; frame < (playbackTickData.Length - 1); frame++)
	{
		ReplayTickData TickData;
		playbackTickData.GetArray(frame, TickData);
		float dist = GetVectorDistance(fClientPos, TickData.origin, true);
		if(dist < fMinDist)
		{
			fMinDist = dist;
			iClosestFrame = frame;
		}
	}
	PrintToServer("暴力查询结果 : %i  精度损失 %i tick", iClosestFrame, iClosestFrame - tick);
	*/
}
