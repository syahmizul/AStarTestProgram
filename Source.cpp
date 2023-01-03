#include <iostream>
#include <vector>
#define GLOBALMAP_WIDTH 5
#define GLOBALMAP_HEIGHT 5
bool GlobalMap[GLOBALMAP_WIDTH][GLOBALMAP_HEIGHT]
{

	{0,0,0,0,0},
	{0,1,1,1,1},
	{0,0,0,0,0},
	{1,1,1,1,0},
	{0,0,0,0,0}
	
};
class Node {
public:

	Node* parent;
	float x, y;
	// Distance From Starting Node
	float g; 
	// Heuristics - Distance From End Node
	float h;
	// Total of g and h
	float f;
	bool IsTraversable;
	Node()
	{
		parent = nullptr;
		x = y = 0;
		f = g = h = 0.0f;
		IsTraversable = false;
	}
};

class NodeMap {
public:
	int width, height;
	uint32_t Map;// TODO: use proper storage next time instead of doing raw pointer arithmetic
	template <int W, int H> static NodeMap Create()
	{
		NodeMap newNodeMap;
		newNodeMap.Map = (uint32_t)(new Node[W][H]);
		newNodeMap.width = W;
		newNodeMap.height = H;
		return newNodeMap;
	}
	Node* GetNode(int x, int y)
	{
		if (x >= width || x < 0 || y >= height || y < 0)
		{
			return nullptr;
		}
		return (Node*)(Map + (x * width * sizeof(Node)) + (y * sizeof(Node)));
	}
	void ImportArray(uint32_t MapArray)
	{

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				
				bool CanTraverse = *(bool*)(MapArray + (x * width * sizeof(bool)) + (y * sizeof(bool)));
				Node newNode;

				newNode.x = (float)x;
				newNode.y = (float)y;
				newNode.IsTraversable = !CanTraverse;

				/*if (newNode.IsTraversable)*/
				/*std::cout << "CanTraverse : " << CanTraverse << "  : " << newNode.x << " " << newNode.y << std::endl ;*/
				*(Node*)(Map + (x * width * sizeof(Node)) + (y * sizeof(Node))) = newNode;
				
			}
			/*std::cout << std::endl;*/
		}
	}

};

float CalculateDistance(Node* Node1, Node* Node2)
{
	float Vector[2]{ Node1->x - Node2->x,Node1->y - Node2->y };
	return sqrt((Vector[0] * Vector[0]) + (Vector[1] * Vector[1]));
}

Node* FindLowestScoreInList(std::vector<Node*>* List)
{
	float f = INFINITY;
	Node* ReturnedNode = nullptr;
	for (Node* node : *List)
	{
		if (node->f <= f)
		{
			f = node->f;
			ReturnedNode = node;
		}
	}
	return ReturnedNode;
}

std::vector<Node*> GenerateNeighbors(NodeMap* _NodeMap,Node* CenterNode)
{
	std::vector<Node*> NeighborList;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x == 0 && y == 0) continue;
			Node* LoopNode = _NodeMap->GetNode(CenterNode->x + x, CenterNode->y + y);
			if (LoopNode)
			{
				NeighborList.push_back(LoopNode);
				//std::cout << LoopNode->x << " " << LoopNode->y << std::endl;
			}
		}
	}
	return NeighborList;
}

void RemoveNodeFromList(std::vector<Node*>* List, Node* node)
{
	for (int i = 0;i<List->size();i++)
	{
		//std::cout << "Removing Node\n";
		Node* nodeInList = List[0][i];
		if (nodeInList == node)
			List->erase(List->begin()+i);
	}
}

bool IsNodeInList(std::vector<Node*>* List, Node* node)
{
	for (Node* ListNode : *List)
	{
		if (node == ListNode)
			return true;
	}
	return false;
}

int main() 
{
	NodeMap nodeMap = NodeMap::Create<GLOBALMAP_WIDTH, GLOBALMAP_HEIGHT>();
	nodeMap.ImportArray((uint32_t)GlobalMap);

	std::vector<Node*> OpenList;
	std::vector<Node*> ClosedList;

	Node* TargetNode	= nodeMap.GetNode(0,4);
	Node* StartingNode	= nodeMap.GetNode(4,0);
	StartingNode->g = CalculateDistance(StartingNode, StartingNode);
	StartingNode->h = CalculateDistance(StartingNode, TargetNode);
	StartingNode->f = StartingNode->g + StartingNode->h;
	OpenList.push_back(StartingNode);
	
	bool IsAtTarget = false;
	std::vector<Node*> Path;
	while (!IsAtTarget)
	{
		Node* CurrentNode = FindLowestScoreInList(&OpenList);
		if (!CurrentNode)
		{
			std::cout << "Can't find path!\n";
			break;
		}
		std::cout << "Current Node Position : " << CurrentNode->x << " " << CurrentNode->y << std::endl;
		RemoveNodeFromList(&OpenList, CurrentNode);
		ClosedList.push_back(CurrentNode);

		if (CurrentNode == TargetNode)
		{
			std::cout << "Arrived at target : " << CurrentNode->x << " " << CurrentNode->y << std::endl;
			IsAtTarget = true;
			Path.push_back(TargetNode);
			for (Node* parentNode = CurrentNode->parent; parentNode != StartingNode; parentNode = parentNode->parent)
			{
				Path.push_back(parentNode);
				/*std::cout << "Parent Node Position : " << parentNode->x << " " << parentNode->y << std::endl;*/
			}
			break;
		}
		std::vector<Node*> NeighborList = GenerateNeighbors(&nodeMap, CurrentNode);

		if (NeighborList.size() < 1)
		{
			std::cout << "No neighbor found" << std::endl;
			break;
		}

		for (Node* NeighborNode : NeighborList)
		{
			/*if (NeighborNode->IsTraversable)
			{
				std::cout << "Current Node Center Position : " << CurrentNode->x << " " << CurrentNode->y << std::endl;
				std::cout << "Neighbor Node : X = " << NeighborNode->x << " Y = " << NeighborNode->y << std::endl;
			}*/
				
			if (!NeighborNode->IsTraversable || IsNodeInList(&ClosedList, NeighborNode))
				continue;

			float GScore = CurrentNode->g + CalculateDistance(NeighborNode, CurrentNode);
			float HScore = CalculateDistance(NeighborNode, TargetNode);
			float FScore = GScore + HScore;

			if (IsNodeInList(&OpenList, NeighborNode) && (NeighborNode->f > FScore))
			{
				NeighborNode->parent = CurrentNode;
				NeighborNode->g = CurrentNode->g + CalculateDistance(NeighborNode, CurrentNode);
				NeighborNode->h = CalculateDistance(NeighborNode, TargetNode);
				NeighborNode->f = NeighborNode->g + NeighborNode->h;
				continue;
			}

			if (!IsNodeInList(&OpenList, NeighborNode))
			{
				NeighborNode->parent = CurrentNode;
				NeighborNode->g = CurrentNode->g + CalculateDistance(NeighborNode, CurrentNode);
				NeighborNode->h = CalculateDistance(NeighborNode, TargetNode);
				NeighborNode->f = NeighborNode->g + NeighborNode->h;
				OpenList.push_back(NeighborNode);
				continue;
			}

		}
			
	}
	
	if (Path.size() < 1)
	{
		std::cout << "Path not found!\n";
		return 0;
	}
	std::reverse(Path.begin(), Path.end());
	std::cout << "Path to destination : \n";
	for (Node* PathNode : Path)
	{
		std::cout << PathNode->x << " " << PathNode->y << std::endl;
	}
}