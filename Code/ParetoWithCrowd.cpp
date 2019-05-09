// Abolfazl Asudeh -- a.asudeh@gmail.com
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <new>

using namespace std;

// Global Variables
int CrowdReq = 0;
int NumberofItems = 0;
int dimensionsize;
short dominateType; // we do not use this any more
short applyCandidateQuestion = 1;
short applyMacroOrdering = 1;
short ***comparisonResults;// this is used when simulation is not used and contains the Pre-asked user inputs (the hidden information)
bool UseHiddenValue; // for the simulation set it to true
short ***dcompared; // the table of comparisons, it is showing the current information about the relations between the objects
/* Possible values for dcompared[i][j][k]:
0: do not dominate each other
1: first dominates the second
-1: second dominates the first
-2: not compared yet
*/


short **compared; // this basically is saying if the domination between two objects are produced (if they are compared)
short * dominated; // 1: this object is dominated. 0: Pareto-optimal candidate. 2: Pareto-optimal
class Item;
Item *items;
vector<int> highranks;


class Item{
public:
	string name;
	int dimensionsize;
	double* hiddenValue;
	int dominationRank;
	int* q1; // # of items this item is better on each dimension
	int* q2; // # of items this and that item were ~ on each dimension
	int* q3; // # of items this item is worse on each dimension
	int* s; // the sorted indices of columns: used in randomized algorithm
	void Init(string Name, int dimensionlenght)
	{
		name = Name;
		hiddenValue = new double[dimensionlenght];
		dimensionsize = dimensionlenght;
		dominationRank = 0;
		q1 = new int[dimensionlenght];
		q2 = new int[dimensionlenght];
		q3 = new int[dimensionlenght];
		s = new int[dimensionlenght]; // used in SLP for selecting the columns
		for (int i = 0; i<dimensionsize; i++) // set the inital order for column selection
		{
			s[i] = i;
			q1[i] = q2[i] = q3[i] = 0;
		}
	}
	Item(){}
	Item(string Name, int dimensionlenght)
	{
		Init(Name, dimensionlenght);
	}

	~Item() {
		delete[] q1;
		delete[] q2;
		delete[] q3;
		delete[] hiddenValue;
	}
};

// Function Prototypes
vector<string> split(string input, char separator)
{
	vector<string> output;
	int i = 0, j;
	while ((j = input.find_first_of(separator, i)) != string::npos)
	{
		output.push_back(input.substr(i, j - i));
		i = j + 1;
	}
	output.push_back(input.substr(i, input.length() - i));
	return output;
}

void ReadItems_hiddenval(const char* input, char separator, int n/*# of items to read*/, string dimensions[], int dimensionindices[], int nameIndex);
void ReadItems_real(int n/*# of items to read*/);
void init();
void reset();
void dispose();
short Crowdcompare(int first, int second, int dimensionIndex); // CrowdFunction
vector<int> FindSkylines(int selectedAlg);
vector<int> Skylines_RandQuestion();
vector<int> Skylines_RandPair();
vector<int> Skylines_RandPair2();
vector<int> Skylines_Sequentional();
void Skylines_SLP();
bool verify(int i, int selectedAlg);//verify that an skyline candidate i is not dominated by any other object
short checkDominance(int i, int j);//checks the dominance status of i , j based on current knowledge:
void Update(int first, int second, int dimension, short outcome);
int RandQuestion(int, int, int dimensionsize);
int SLPQuestion(int first, int second);
int Dominated(int, int, int dimensionsize);
int Dominated00(int, int, int dimensionsize);
int Dominated10(int, int, int dimensionsize);
void LP(int first/*input*/, int& second/*output*/, int& length/*output*/, bool *firstcomp/*init by true*/); // computes and returns the lucky path of item i
void SLP(int& first/*output*/, int& second/*output*/, int& length/*output*/, bool *firstcomp/*init by true*/); // computes and returns the shortest lucky path
void sortCols(int i); // used in Dominated10, useless every where else
int ColSortFunc(int i, int d);
void UpdateHighranks(int i, int j, int k);
void SortItems();
int SortUnkowns4SLP(int x, int y);
int find(int key);


const char* dFile(int k) // used for reading real crowd results
{
	switch (k)
	{
	case 0:
		return "D0.csv";
	case 1:
		return "D1.csv";
	case 2:
		return "D2.csv";
	case 3:
		return "D3.csv";
	case 4:
		return "D4.csv";
	case 5:
		return "D5.csv";
	case 6:
		return "D6.csv";
	case 7:
		return "D7.csv";
	default:
		return "D8.csv";
	}
}


int main(int argc, char* argv[])
{
	if (argc<4) { cout << argv[0] << "#dimension #objects Alg filenameIndex applyCandidate(0/1) applyMacroOrdering(0/1) [4 100 3 1 0 0]" << endl; return 0; }
	int n, i, j, k, l, failcount;
	const char* filename;
	char separator;
	dominateType = 1;
	int selectedAlg = 0;
	dimensionsize =  atoi(argv[1]);
	n = atoi(argv[2]);
	selectedAlg = atoi(argv[3]);
	if(argc>4)
        switch (atoi(argv[4]))
        {
        case 1:
            filename = "independent_10_10000.txt";
            separator = ' ';
            break;
        case 2:
            filename = "correlated_10_10000.txt";
            separator = ' ';
            break;
        case 3:
            filename = "Anticorrelated_10_10000.txt";
            separator = ' ';
            break;
        default:
            filename = "player_regular_season.csv";//for NBA DataSet
            separator = ',';
            break;
        }
	if (argc == 7)
	{
		applyCandidateQuestion = atoi(argv[5]);
		applyMacroOrdering = atoi(argv[6]);
	}
	else
	{
		applyCandidateQuestion = 1;
		applyMacroOrdering = 1;
	}


	/*cout<<endl<<"Enter Selected Algorthim:";
	cin>>selectedAlg;
	cout<<endl<<"Enter Number of items:";
	cin>>n;
	*/
	//UseHiddenValue=false;
	UseHiddenValue = true;
	int nameindex = 0; //for NBA DataSet
	//int dimensionIndices[10] = { 8, 11, 12, 9, 10, 14, 16, 17, 18, 19 }; //for NBA DataSet
	int dimensionIndices[10] = {0,1,2,3,4,5,6,7,8,9}; // for synthetic data
	string dimensions[10] = { "Points", "Rebounds", "Assists", "dreb", "reb", "blk", "pf", "fga", "fgm", "fta" };//for NBA dataset
	vector<int> skylines;
	string temp;
	if (UseHiddenValue)
		ReadItems_hiddenval(filename,separator, n, dimensions, dimensionIndices, nameindex);
	else
		ReadItems_real(n);

	/*for(i=0;i<n;i++)
		{
		    for(j=0;j<dimensionsize;j++)
                cout<<"\t"<<items[i].hiddenValue[j];
            cout<<endl;
		}*/
	init();
	skylines = FindSkylines(selectedAlg);



	//for(i=0;i<skylines.size();i++)
	//cout<<endl<<i<<", "<<items[skylines[i]].name;
	//cout<<endl<<"CrowdQuestions:"<<CrowdReq << endl;
	cout << endl<<selectedAlg << ',' << dimensionsize << ',' << n << ',' << CrowdReq << ',' << skylines.size() << endl;
	//_getch();
	return 0;
}

void Update(int first, int second, int d/*dimension*/, short o/*outcome*/) // this function updates R^+
{
	int i, j, n = NumberofItems;
	if (o == 1) // first is better on dimension:i
	{
		for (i = 0; i<n; i++) // for all items
		{
			if (dcompared[second][i][d] == 1)
			{
				dcompared[first][i][d] = 1;
				dcompared[i][first][d] = -1;
			}
			if (dcompared[first][i][d] == -1)
			{
				dcompared[second][i][d] = -1;
				dcompared[i][second][d] = 1;
			}
		}
		dcompared[first][second][d] = 1;
		dcompared[second][first][d] = -1;
	}
	else if (o == -1) // second is better on dimension:i
	{
		for (i = 0; i<n; i++) // for all items
		{
			if (dcompared[first][i][d] == 1) //children of first
			{
				dcompared[second][i][d] = 1;
				dcompared[i][second][d] = -1;
			}
			if (dcompared[second][i][d] == -1) // parents of second
			{
				dcompared[first][i][d] = -1;
				dcompared[i][first][d] = 1;
			}
		}
		dcompared[first][second][d] = -1;
		dcompared[second][first][d] = 1;
	}
	else dcompared[first][second][d] = dcompared[second][first][d] = 0;
}

short Crowdcompare(int first, int second, int dimensionIndex) // CrowdFunction
{
	if (dcompared[first][second][dimensionIndex] != -2)
		return dcompared[first][second][dimensionIndex];
	CrowdReq++;
	//cout<<endl<<items[first].name<<","<<items[second].name<<","<<dimensionIndex<<",";
	short returnVal;
	if (UseHiddenValue)
	{

		double diff = (items[first].hiddenValue[dimensionIndex] - items[second].hiddenValue[dimensionIndex]);
		double p = ((double)rand()) / (RAND_MAX);
		double th = 1 - exp(-abs(diff * 10.) + log(0.5));

		if (p > th) { //reverse order
			if (diff >= 0) {
				returnVal = (p>(1 + th) / 2) ? -1 : (diff == 0 ? 1 : 0);
			}
			else {
				returnVal = (p>(1 + th) / 2) ? 1 : 0;
			}
		}
		else {
			returnVal = diff < 0 ? -1 : (diff > 0 ? 1 : 0); //original order
		}

/*		
		if(items[first].hiddenValue[dimensionIndex]>items[second].hiddenValue[dimensionIndex])
			returnVal = 1; //first is better
		else if(items[first].hiddenValue[dimensionIndex]<items[second].hiddenValue[dimensionIndex])
			returnVal = -1; // second is better
		else 
			returnVal = 0; // indifferent or incomparable
*/	
	}
	else
	{
		returnVal = comparisonResults[first][second][dimensionIndex];
	}
	//cout << first << ", " << second << ", "<< dimensionIndex << ", " << returnVal << endl;
	if (applyCandidateQuestion==1)
		Update(first, second, dimensionIndex, returnVal);
	else
	{
			dcompared[first][second][dimensionIndex] = returnVal;
			dcompared[second][first][dimensionIndex] = -1 * returnVal;
	}
	return returnVal;
}

void ReadItems_hiddenval(const char* input, char separator, int n/*# of items to read*/, string dimensions[], int dimensionindices[], int nameIndex)
{
	int i;
	Item tmpItem(string("non"), dimensionsize);
	vector<string> tmpvectorString;
	ifstream infile;
	infile.open(input, std::ifstream::in);
	if (!infile.is_open())
	{
		cout << "could not open the input file";
		exit(0);
	}
	//cout<<endl<<"opened the file: "<<input<<", separator="<<separator;
	string temp;
	//skip the header
	getline(infile, temp);

	// reading the items from the file
	items = new Item[n];
	while (NumberofItems<n && getline(infile, temp))
	{
		// fetch the current item
		tmpvectorString = split(temp, separator); // IMPORTANT: it works just for CVS files
		// check if this line is complete
		if (tmpvectorString.size() <= dimensionindices[dimensionsize - 1]) // I assumed the last dimension is has the biggest index
			//cout<<endl<<"ERROR!: line "+temp+ " does not have enough DIMENSIONS!";
		{
		}
		else
		{
			items[NumberofItems].Init(tmpvectorString[nameIndex], dimensionsize);
			for (i = 0; i<dimensionsize; i++)
				items[NumberofItems].hiddenValue[i] = atof(tmpvectorString[dimensionindices[i]].c_str());
			NumberofItems++;
		}
	}
	infile.close();
	n = NumberofItems;
}

void ReadItems_real(int n/*# of items to read*/)
{
	// Read the list of items from the input file
	int i, j, k;
	NumberofItems = n;
	vector<string> tmpvectorString;
	items = new Item[NumberofItems];
	ifstream infile;
	infile.open("Objects.txt", std::ifstream::in);
	if (!infile.is_open())
	{
		cout << "could not open Objects.txt";
		exit(0);
	}
	string temp;
	// reading the items from the file
	for (i = 0; i<NumberofItems && getline(infile, temp); i++)
	{
		items[i].Init(temp, dimensionsize);
	}
	infile.close();
	NumberofItems = i;

	//create and fill the comparisons results
	comparisonResults = new short**[NumberofItems];
	for (i = 0; i<NumberofItems; i++)
	{
		comparisonResults[i] = new short*[NumberofItems];
		for (j = 0; j<NumberofItems; j++)
			comparisonResults[i][j] = new short[dimensionsize];
	}

	for (k = 0; k<dimensionsize; k++)
	{
		infile.open(dFile(k), std::ifstream::in);
		if (!infile.is_open())
		{
			cout << "could not open One of dimension files";
			exit(0);
		}
		// reading the results from the file
		for (i = 0; i<NumberofItems && getline(infile, temp); i++)
		{
			tmpvectorString = split(temp, ','); // IMPORTANT: it works just for CSV files
			for (j = 0; j<NumberofItems; j++)
				comparisonResults[i][j][k] = atof(tmpvectorString[j].c_str());
		}
		infile.close();
	}
}

void init()
{
	int i, j, k, n = NumberofItems;
	compared = new short*[n];
	dcompared = new short**[n];
	dominated = new short[n];
	for (i = 0; i<n; i++)
	{
		compared[i] = new short[n];
		for (j = 0; j<n; j++)
			compared[i][j] = 0;
		compared[i][i] = 1; // not to compare an item with itself
		dominated[i] = 0;
		dcompared[i] = new short*[n];
		for (j = 0; j<n; j++)
		{
			dcompared[i][j] = new short[dimensionsize];
			for (k = 0; k<dimensionsize; k++)
				dcompared[i][j][k] = -2; // not compared yet
		}
		for (k = 0; k<dimensionsize; k++)
			dcompared[i][i][k] = -3; // this default value means do not compare an item with itself
	}
	CrowdReq = 0;
}

void reset()
{
	int i, j, k, n = NumberofItems;
	for (i = 0; i<n; i++)
	{
		for (j = 0; j<n; j++)
			compared[i][j] = 0;
		compared[i][i] = 1; // not to compare an item with itself
		dominated[i] = 0;
		for (j = 0; j<n; j++)
		{
			for (k = 0; k<dimensionsize; k++)
				dcompared[i][j][k] = -2; // not compared yet
		}
		for (k = 0; k<dimensionsize; k++)
			dcompared[i][i][k] = -3; // this default value means do not compare an item with itself
	}
	CrowdReq = 0;
	highranks.clear();
}

void dispose()
{
	NumberofItems = 0;
	CrowdReq = 0;
	delete[] items;
	delete[] dcompared;
	delete[] compared;
	delete[] dominated;
	if (!UseHiddenValue) delete[] comparisonResults;
	highranks.clear();
}

short checkDominance(int first, int second)
{
	//  0: do not dominate each other or indifferent
	//  1: first dominates the second
	// -1: second dominates first
	// -2: second cannot dominate first (first is better in at least one dimension)
	// -3: first cannot dominate second (second is better in at least one dimension)
	// -4: both have the chance to dominate each other
	int i;
	bool fd = true, sd = true;
	for (int i = 0; i<dimensionsize; i++)
	{
		if (dcompared[first][second][i] == 1)//first is better on i
			sd = false;
		else if (dcompared[first][second][i] == -1)//second is better on i
			fd = false;
		if (!fd && !sd)
			return 0; //first and second do not dominate each other
	}
	for (i = 0; i<dimensionsize; i++)
		if (dcompared[first][second][i] == -2)
		{
		if (fd) return -2;
		if (sd)return -3;
		return -4;
		}
	if(fd && sd) return 0;
	if (fd) return 1;
	if (sd)return -1;
	return 0; // those are indifferent in all dimensions
}

bool verify(int i, int selectedAlg)
{
	int j, k, d, o;
	bool fdominated = true;
	bool sd = false;

	vector<int> checklist, dimensions;
	for (j = 0; j<NumberofItems; j++) {
		checklist.push_back(j);
	}

	for (d = 0; d<dimensionsize; d++)
		dimensions.push_back(d);

	if (selectedAlg == 0 || selectedAlg == 1) {
		//randomize it for random pair/random question
		random_shuffle(checklist.begin(), checklist.end());
		for (vector<int>::iterator it = checklist.begin(); it != checklist.end(); ++it) {
			j = *it;
			if (dominated[j] == 1) // the item is dominated
			{
				k = checkDominance(i, j);
				//if(k==0)//  0: do not dominate each other or indifferent --> no furthur question is needed, just return true
				//	RETURN true;
				//if(k==1)//  1: first dominates the second --> no furthur question is needed, , just return true
				//	RETURN true;
				if (k == -1)// -1: second dominates first --> no furthur question is needed, just return false
					return false;
				//if(k==-2)// -2: second cannot dominate first (first is better in at least one dimension) --> no furthur question, just return true
				//	RETURN true;
				if (k == -3)// -3: first cannot dominate second (second is better in at least one dimension) --> there is a chance that second can dominate first
				{
					// ask question until in one dimension first is better (return true in this case) or second dominate first (return false)
					if (selectedAlg == 0) // random question: change it random
						random_shuffle(dimensions.begin(), dimensions.end());

					for (vector<int>::iterator dit = dimensions.begin(); dit != dimensions.end(); ++dit) {
						d = *dit;
						if (dcompared[i][j][d] == -2)
						{
							o = Crowdcompare(i, j, d);
							if (o == 1){ fdominated = false; break; }
						}
					}
					if (fdominated)
						return false;
					fdominated = true;
				}
				if (k == -4)// -4: both have the chance to dominate each other --> continue questioning
				{
					if (selectedAlg == 0) // random question: change it random
						random_shuffle(dimensions.begin(), dimensions.end());

					for (vector<int>::iterator dit = dimensions.begin(); dit != dimensions.end(); ++dit) {
						d = *dit;
						if (dcompared[i][j][d] == -2)
						{
							o = Crowdcompare(i, j, d);
							if (o == 1){ fdominated = false; break; }
							if (o == -1) sd = true;
						}
						if (!fdominated)
						{
							if (sd == true)
								return false;
							sd = false;
						}
						fdominated = true;
					}
				}
			}
		}
	}
	else if (selectedAlg == 3) {
		/*
		Find a object y in dominated array in which value = 1, and no criteron c s.t. x >c y, select the y with highest dominationRank
		select the criterion that dcompared[x][y][c]==0, and max of ColSortFunc(y, c)
		*/
		bool jci;  // it is used to check if exists c such that y=items[j] >c x=items[i]
		do {
			j = -1;
			double maxRank = -1;
			for (int ci = 0; ci<checklist.size(); ++ci) {
				int __j = checklist[ci];
				if (dominated[__j] == 1) {
					bool xcy = false;
					bool ycx = false;
					bool __c__ = false;
					for (d = 0; d<dimensionsize; d++) {
						if (dcompared[i][__j][d] == 1) {
							xcy = true;
							break;
						}
						else if (dcompared[i][__j][d] == -1) ycx = true;
						else if (dcompared[i][__j][d] == -2) {
							__c__ = true;
						}
					}

					if (!xcy && ycx && !__c__) // y dominated x
						return false;

					if (!xcy && __c__ && maxRank < items[__j].dominationRank) {
						maxRank = items[__j].dominationRank;
						j = __j;
						jci = ycx;
					}
				}
			}

			if (j == -1) break; // there is no item y that can dominate x: x is Pareto-optimal for sure

			sortCols(j);
			bool icj = false;
			for (int dim = 0; dim<dimensionsize; dim++)
			{
				d = items[j].s[dim];
				if (dcompared[i][j][d] == -2)
				{
					Crowdcompare(i, j, d);
					if (dcompared[i][j][d] == 1)
					{
						icj = true;
						break;
					}
					else if (dcompared[i][j][d] == -1) jci = true;
				}
			}

			if (jci &&!icj) // j dominated i
				return false;
		} while (1);
	}

	return true;
}

bool itemDRank(int i, int j) {
	return items[i].dominationRank < items[j].dominationRank;
}

vector<int> FindSkylines(int selectedAlg)
{
	vector<int> candidates, skylines;
	switch (selectedAlg)
	{
	case 0:
			candidates = Skylines_RandQuestion();
		random_shuffle(candidates.begin(), candidates.end());
		break;
	case 1:
		candidates = Skylines_RandPair();
		random_shuffle(candidates.begin(), candidates.end());
		break;
	case -1:
		candidates = Skylines_RandPair2();
		break;
	case 2:
		candidates = Skylines_Sequentional();
		break;
	default: //case 3:
		Skylines_SLP();
		candidates = highranks;
		// Find a object x in candidates which dominates least #objects ==> dominationRank
		sort(candidates.begin(), candidates.end(), itemDRank);
	}

	for (int i = 0; i<candidates.size(); i++)  //randomize it for random pair/random question
		if (verify(candidates[i], selectedAlg))
			skylines.push_back(candidates[i]);

	return skylines;
}

vector<int> Skylines_RandQuestion()
{
	int i, j, k;
	int n = NumberofItems;
	srand((unsigned)time(0));
	//finding skylines
	bool valid, done = false;
	do{
		do{
			//select the random item first
			i = rand() % n; k = i;
			while (dominated[i])
			{
				i = (i + 1) % n;
				if (i == k){ done = true; break; }
			}
			if (!done) // there still is an item that is not dominated nor compared with all items
			{
				if (applyCandidateQuestion == 0) i = k;
				//select a random item second
				j = rand() % n; k = j; valid = true;
				while (compared[i][j]|| (applyMacroOrdering==1 && dominated[j] == 1))
				{
					j = (j + 1) % n;
					if (k == j)
					{
						if(dominated[i]==0) dominated[i] = 2; // skyline for sure.
						valid = false;
						break;
					}
				}
			} // end if(!done)
		} while (!done && !valid);
		if (done) break;
		k = RandQuestion(i, j, dimensionsize);
		if (k != -2) // the result of dominasion is clear for these items
		{
			compared[i][j] = compared[j][i] = 1;
			if (k == 1) // i dominated j
			{
				dominated[j] = 1;
			}
			else if (k == -1) // j dominated i
			{
				dominated[i] = 1;
			}
		}

	} while (!done);

	//fill the skyline items
	vector<int> skylines;
	for (i = 0; i<n; i++)
		if (dominated[i] == 2)
			skylines.push_back(i);
	return skylines;
}


vector<int> Skylines_RandPair()
{
	int i, j, k;
	int n = NumberofItems;
	//finding skylines
	srand((unsigned)time(0));
	bool valid, done = false;
	do{
		do{
			//select the random item first
			i = rand() % n; k = i;
			while (dominated[i]) // find x such that x belongs to S_?
			{
				i = (i + 1) % n;
				if (i == k){ done = true; break; }
			}
			if (!done) // there still is an item that is not dominated nor compared with all items
			{
				//select a random item second
				j = rand() % n; k = j; valid = true;
				while (dominated[j] == 1 || compared[i][j]) // find y such that y belongs to S_\surd or S_?
				{
					j = (j + 1) % n;
					if (k == j)
					{
						dominated[i] = 2; // skyline for sure.
						valid = false;
						break;
					}
				}
			} // end if(!done)
		} while (!done && !valid);
		if (done) break;

		k = Dominated00(i, j, dimensionsize);
	} while (!done);

	//fill the skyline items
	vector<int> skylines;
	for (i = 0; i<n; i++)
		if (dominated[i] == 2)
			skylines.push_back(i);
	return skylines;
}

vector<int> Skylines_RandPair2() // delete later
{
	int i, j, k;
	int n = NumberofItems;
	//finding skylines
	srand((unsigned)time(0));
	bool valid, done = false;
	do{
		do{
			//select the random item first
			i = rand() % n; k = i;
			while (dominated[i] != 0 && dominated[i] != 1)
			{
				i = (i + 1) % n;
				if (i == k){ done = true; break; }
			}
			if (!done) // there still is an item that is not dominated nor compared with all items
			{
				//select a random item second
				j = rand() % n; k = j; valid = true;
				while (compared[i][j])
				{
					j = (j + 1) % n;
					if (k == j)
					{
						if (dominated[i] == 0)
							dominated[i] = 2; // skyline for sure.
						else
							dominated[i] = 3; // dominated for sure.
						valid = false;
						break;
					}
				}
			} // end if(!done)
		} while (!done && !valid);
		if (done) break;

		k = Dominated00(i, j, dimensionsize);
	} while (!done);

	//fill the skyline items
	vector<int> skylines;
	for (i = 0; i<n; i++)
		if (dominated[i] == 2)
			skylines.push_back(i);
	return skylines;
}

vector<int> Skylines_Sequentional()
{
	int i, j, k;
	int n = NumberofItems;
	int tmp;
	vector<int> skylines;
	// finding the skylines
	for (int current = 0; current<n; current++)
	{
		bool d = false;
		for (i = 0; i<skylines.size(); i++)
		{
			j = Dominated00(current, skylines[i], dimensionsize);
			if (j == 1) // the current Item dominates the skyline[i]
			{
				skylines.erase(skylines.begin() + i);
			}
			else if (j == -1)
			{
				d = true;
				break;
			}
		}
		if (!d)
			skylines.push_back(current);
	}
	return skylines;
}

void Skylines_SLP()
{
	int i, j, k, length;
	int n = NumberofItems;
	//finding skylines
	bool *firstcomp = new bool[n];
	for (i = 0; i<n; i++) firstcomp[i] = true;
	vector<int> highranks;

	bool valid, done = false; // delete
	do{
		SLP(i, j, length, firstcomp);
		if (i == -1 || j == -1) break; // none of the objects in $X$ dominate each other
		firstcomp[i] = false;
		firstcomp[j] = false;
		k = SLPQuestion(i, j);
		//update highranks
		UpdateHighranks(i, j, k);
		SortItems();
	} while (i != -1);
	//fill the skyline items
}

int SLPQuestion(int first, int second)
{
	/* The outputs:
	0: do not dominate each other
	1: first dominates the second
	-1: second dominates the first
	-2: keep going to ask questions about these two items
	*/
	int d, dim, k;
	//find a dimension:i to ask the question about
	// MAKE SURE THERE IS AT LEAST ONE MORE DIMENSION TO ASK THE QUESTION
	for (d = 0; d<dimensionsize; d++)
		if (dcompared[first][second][d] == -2)
		{
		dim = d;
		d++;
		break;
		}
	for (; d<dimensionsize; d++)
		if (dcompared[first][second][d] == -2)
			if (ColSortFunc(second, dim)<ColSortFunc(second, d))
				dim = d;
	//ask the question on found dimension
	k = Crowdcompare(first, second, dim);
	//produce the outputs
	k = checkDominance(first, second);
	if (k == -1)//second dominated the first
	{
		compared[first][second] = 1;
		dominated[first] = 1;
		return -1;
	}
	if (k == 1)//first dominated the second
	{
		compared[first][second] = 1;
		dominated[second] = 1;
		return 1;
	}
	if (k == 0)//do not dominate each other
	{
		compared[first][second] = 1;
		return 0;
	}
	return -2;
}

int RandQuestion(int first, int second, int dsize)
{
	/* The outputs:
	0: do not dominate each other
	1: first dominates the second
	-1: second dominates the first
	-2: keep going to ask questions about these two items on this dimension
	*/
	int i, j, k;
	bool ask = true;
	//find a dimension:i to ask the question about
	i = rand() % dsize; k = i;
	while (dcompared[first][second][i] != -2) //IMPORTANT: I have to make sure that there still is at least one more dimension to ask
	{
		i = (i + 1) % dsize;
		if (i == k)//these two objects are have not any more comparisions to do
		{
			ask = false;
			break;
		}
	}
	//ask the question on found dimension
	if (ask)
		k = Crowdcompare(first, second, i);
	//produce the outputs
	bool fd = true, sd = true;
	for (i = 0; i<dsize; i++)
	{
		if (dcompared[first][second][i] == 1)//first is better on i
			sd = false;
		else if (dcompared[first][second][i] == -1)//second is better on i
			fd = false;
		if (!fd && !sd && (applyCandidateQuestion==1))
			return 0; //first and second do not dominate each other
	}
	for (i = 0; i<dsize; i++)
		if (dcompared[first][second][i] == -2)
			return -2; // continue on these two items
	if (fd && !sd) return 1;
	if (sd && !fd)return -1;
	return 0; // those are indifferent in all dimensions
}

int Dominated00(int first, int second, int dimensionsize)
{
	int i, j, d, k;
	compared[second][first] = compared[first][second] = 1;
	bool fd = true, sd = true; // fd: First Dominates the Socnd one; sd: Socond Dominates the First One
	bool indifferent = true;
	for (i = 0; i<dimensionsize; i++)
	{
		k = Crowdcompare(first, second, i);
		if (k == 1) // first item dominates the  second one on this dimension
		{
			if (!fd) return 0; // none dominates the other
			sd = false;
			indifferent = false;
		}
		else if (k == -1)
		{
			if (!sd) return 0; // none dominates the other
			fd = false;
			indifferent = false;
		}
	}
	if (indifferent == true) return 0;
	if (sd)
	{
		dominated[first] = 1;
		return -1;
	}
	dominated[second] = 1;
	return 1;
}

int Dominated10(int first, int second, int dimensionsize){

	int i, j, d, k;
	bool fd = true, sd = true; // fd: First Dominates the Socnd one; sd: Socond Dominates the First One
	compared[second][first] = compared[first][second] = 1;
	bool indifferent = true;
	for (d = 0; d<dimensionsize; d++)
	{
		i = items[second].s[d]; // next dimension to ask the question about
		k = Crowdcompare(first, second, i);
		if (k == 1) // first item dominates the  second one on this dimension
		{
			items[first].q1[i]++;
			items[second].q3[i]++;
			if (!fd)
			{
				sortCols(first);
				sortCols(second);
				return 0; // none dominates the other
			}
			sd = false;
			indifferent = false;
		}
		else if (k == -1)
		{
			items[first].q3[i]++;
			items[second].q1[i]++;
			if (!sd)
			{
				sortCols(first);
				sortCols(second);
				return 0; // none dominates the other
			}
			fd = false;
			indifferent = false;
		}
		else
		{
			items[first].q2[i]++;
			items[second].q2[i]++;
		}
	}
	if (indifferent == true)
	{
		sortCols(first);
		sortCols(second);
		return 0;
	}
	if (sd)
	{
		sortCols(second);
		dominated[first] = 1;
		return -1;
	}
	sortCols(first);
	dominated[second] = 1;
	return 1;
}

int getRemained(int i, int j) // this function is just used in LP function
{
	int d, sum = 0;
	if (dominated[i] == 1 || dominated[j] == 1){
		//cout<<endl<<"be carefull in sending items to getRemined function!";
		return -1; // IT HAS NOT TO OCCURE AT ALL
	}
	for (d = 0; d<dimensionsize; d++)
		if (dcompared[i][j][d] == -2) sum++;
	return sum;
}

void LP(int first/*input*/, int& second/*output*/, int& length/*output*/, bool *firstcomp/*init by true*/)
{
	int i, d, n = NumberofItems, len, k;
	second = -1; //for the cases that no item is found
	if (firstcomp[first])
	{
		length = dimensionsize;
		if (highranks.size()>0)
			second = highranks[0];
		else
		{
			for (i = 0; i<n; i++)
				if (dominated[i] == 0 && i != first)
				{
				second = i;
				break;
				}
		}
		//*** no worries, firstcomp[first] is changed in slp
		return;
	}

	// check with highranks
	// high ranks are the sorted set of non dominated items that has at least one comparison

	if (highranks.size() == 0) // there's no item that has at least one comparison and has not been dominated yet
	{
		length = dimensionsize;
		for (i = 0; i<n; i++)
			if (firstcomp[i] == true)
			{
			second = i;
			break;
			}
		//cout<<endl<<"Wired!, It has not to happen, in function: LP";
		return;
	}

	for (i = 0; i<highranks.size(); i++)
		if (compared[first][highranks[i]] == 0)
		{
		k = checkDominance(first, highranks[i]);
		if (k == 1 || k == -1)
			UpdateHighranks(first, highranks[i], k); //this looks weird!
		else if (k == 0)
			compared[first][highranks[i]] = 1;
		else
		{
			len = getRemained(first, highranks[i]);
			if (len>0)
			{
				length = len;
				second = highranks[i];
				i++;
				break;
			}
		}
		}
	for (; i<highranks.size(); i++)
		if (compared[first][highranks[i]] == 0)
		{
		k = checkDominance(first, highranks[i]);
		if (k == 1 || k == -1)
			UpdateHighranks(first, highranks[i], k);
		else if (k == 0)
			compared[first][highranks[i]] = 1;
		else
		{
			len = getRemained(first, highranks[i]);
			if (len == 0) compared[first][highranks[i]] = 1;
			if (len>0)
				if (len<length)
				{
				length = len;
				second = highranks[i];
				}
		}
		}
	if (second == -1)
	{
		length = dimensionsize;
		for (i = 0; i<n; i++)
			if (firstcomp[i] == true)
			{
			second = i;
			break;
			}
	}
}

void SLP(int& first/*output*/, int& second/*output*/, int& length/*output*/, bool *firstcomp/*init by true*/)
{
	//it should return first=-1 when there is no more comparison to do
	first = -1;
	int i, n = NumberofItems;
	int sec, len;
	for (i = 0; i<n; i++)
		if (dominated[i] == 0)
		{
		LP(i, sec, len, firstcomp); //if second == -1 it means none of the none-dominated objects dominate i
		if (sec != -1)
		{
			first = i;
			second = sec;
			length = len;
			i++;
			break;
		}
		}

	for (; i<n; i++)
		if (dominated[i] == 0)
		{
		LP(i, sec, len, firstcomp);
		if (sec != -1 && len<length)
		{
			first = i;
			second = sec;
			length = len;
		}
		else if (sec != -1 && len == length)
			if (items[i].dominationRank<items[first].dominationRank)
			{
			first = i;
			second = sec;
			}
		}
}

int ColSortFunc(int i, int d) // just used in srotCols function
{
	//temp=items[i].q1[d]; the best so far
	int temp = items[i].q1[d] + items[i].q3[d];//+items[i].q2[d];
	return temp;
}

void sortCols(int i)
{
	int j, k, l, temp;
	for (j = 0; j<items[i].dimensionsize; j++)
	{
		l = j;
		for (k = j + 1; k<items[i].dimensionsize; k++)
			if (ColSortFunc(i, items[i].s[l]) < ColSortFunc(i, items[i].s[k])) l = k;
		if (l != j)
		{
			temp = items[i].s[l];
			items[i].s[l] = items[i].s[j];
			items[i].s[j] = temp;
		}
	}
}

void UpdateHighranks(int i, int j, int k) //***check highranks to make sure it does not add an item twise, if it already is there
{
	int it;
	if (k == 1)//remove j from highranks; add i to highranks; transfer the rank of j to i
	{
		items[i].dominationRank += (items[j].dominationRank + 1);
		it = find(j);
		if (it != -1)
			highranks.erase(highranks.begin() + it);
		it = find(i);
		if (it == -1)
			highranks.push_back(i);
	}
	else if (k == 0 || k<-1)//add i,j to highranks
	{
		it = find(j);
		if (it == -1)
			highranks.push_back(j);
		it = find(i);
		if (it == -1)
			highranks.push_back(i);
	}
	else if (k == -1)//remove i from highranks; add j to highranks; transfer the rank of i to j
	{
		items[j].dominationRank += (items[i].dominationRank + 1);
		it = find(i);
		if (it != -1)
			highranks.erase(highranks.begin() + it);
		it = find(j);
		if (it == -1)
			highranks.push_back(j);
	}
}

void SortItems() // insertion sort; since we hope the array is almost ranked, insertion sort is almost linear
{
	int i, j, temp;
	for (i = 1; i<highranks.size(); i++)
	{
		j = i - 1;
		temp = highranks[i];
		while (items[highranks[i]].dominationRank>items[highranks[j]].dominationRank)
		{
			highranks[j + 1] = highranks[j];
			j--;
			if (j<0) break;
		}
		if (j<i - 1)
			highranks[j + 1] = temp;
	}
}

int find(int key)
{
	for (int i = 0; i<highranks.size(); i++)
		if (highranks[i] == key) return i;
	return -1;
}
