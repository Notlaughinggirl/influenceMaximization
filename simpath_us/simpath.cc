#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<map>
#include<set>
#include<cstdlib>

using namespace std;

struct Node{
	int id; //�����
	multimap<double,Node*> *inNodes; //�������
	multimap<double,Node*> *outNodes; //�������
	int inDegree; //������
	int outDegree; //������
	bool onCurrentPath; //�Ƿ�����ڵ�ǰ·���С�������Optimizing Simpath
	double pp; //��¼�����ӽ�㵽�ý���·�����ʡ������ڼ���Ӱ�촫��ֵ
	double inf; //��¼�����ӽ�㵽�ý���Ӱ�졪�����ڼ���Ӱ�촫��ֵ
};

/*���ַ���ת��Ϊint*/
inline unsigned int strToInt(string s) {
	unsigned int i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a number." << endl;
		return atoi(s.c_str());
	}
	return i;
}

/*���ַ���ת��Ϊdouble*/
inline double strToDouble(string s) {
    return atof(s.c_str());
    double i;
    istringstream myStream(s);

    if (myStream>>i) {
        return i;
    } else {
        cout << "String " << s << " is not a float." << endl;
		return atof(s.c_str());
    }
    return i;
}

class SimPath{
private:
	map<int,Node*> *nodes; //������н����Ϣ���������ȡ���ȵȣ�
	int edges; //����ߵ�����
	set<int> nodeIds; //������ж���id
	set<int> vcNodeIds; //�����С���㸲�ǵĶ���id
	set<int> nvcNodeIds; //��Ų�����С���㸲���еĶ���id
	set<int> seedNodeIds; //������ӽ��id�����ս��
	map<int,double> nvcInf; //��Ų���VC�еĽ���Ӱ��ֵ
	map<int,double> infSUp; //����simPathSpreadNormal ���inf{V-x}(S) x����U
	map<int,double> infSDown;  //���inf{V-S}(x)
	double pruneVal; //·����֦��ֵ
	int topL; //���ú�ѡ�����Ŀ
	int k; //������Ҫѡ�������ӽڵ㼯��
public:
	SimPath();//���캯��
	void setParameter(double _pruneVal,int _topL, int _k);//���ò���
	void readDataSets(string file); //��ȡ�ļ������㼯��
	void findVertexCover();//��һ�ֵ���ǰѰ����С���㸲��
	void simPathCore(); //�㷨���ģ�����Ѱ��k�����ӽ�㱣����seedNodeIds��
	double simPathSpreadFirst(int id); //��һ�ֵ��������㸲�Ǽ��еĵ�
	double simPathSpreadNormal(set<int> S,set<int> U); //�����ֵ�������Ӱ��ֵInf(S)ͬʱ����Inf{V-x}(S)��x����U
	double backtrackNormal(int id,set<int> S,set<int> U); //����Inf{V-S+u-x}(u),x����U
	double backtrackSimple(int id,set<int> S); //����Inf{V-S}(x)
};

/*���캯��*/
SimPath::SimPath(){
	nodes = new map<int,Node*>();
	pruneVal = 0.001;
	topL = 4;
}

/*���ò���*/
void SimPath::setParameter(double _pruneVal,int _topL, int _k){
	pruneVal = _pruneVal;
	topL = _topL;
	k = _k;
}

/*��ȡ�ļ������㼯��*/
void SimPath::readDataSets(string file){
    cout << "Reading \"" << file << "\"data file... " <<endl;
	ifstream dataFile (file.c_str(), ios::in);

    if (dataFile.is_open()) {
		int id1,id2,b; //��ȡ�����1�������2�Լ�Ӱ������b
		string split = " \t";	

        while (!dataFile.eof() )	{
            string line;
			getline (dataFile,line);
			if (line.empty()) {continue;} //�����������
			edges++;
			if (edges == 0) {continue;} //�����һ������
			string::size_type pos = line.find_first_of(split);
			int	prevpos = 0;

			// ��ȡ��һ�������
			int id1 = strToInt(line.substr(prevpos, pos-prevpos));

			// ��ȡ�ڶ��������
			prevpos = line.find_first_not_of(split, pos);
			pos = line.find_first_of(split, prevpos);
			int id2 = strToInt(line.substr(prevpos, pos-prevpos));

			// ��ȡ��Ȩ��Ӱ������
			double b = 0;
			prevpos = line.find_first_not_of(split, pos);
			pos = line.find_first_of(split, prevpos);
			if (pos == string::npos) 
				b = strToDouble(line.substr(prevpos));
			else
				b = strToDouble(line.substr(prevpos, pos-prevpos));

			// �������Ȼ�b=0��Ϊ��Ч��
			if (id1==id2||b == 0) {
                edges--; 
                continue;
			}

			// ����id��ȡ��ӦNode
            Node *node1 = NULL;
            Node *node2 = NULL;
			
            pair<set<int>::iterator,bool> r1, r2; //��¼�������Ӷ��жϸý���Ƿ���ֹ�
			r1 = nodeIds.insert(id1);
            r2 = nodeIds.insert(id2);
			
            if (r1.second == true) // id1����ɹ���˵�����δ���ֹ�
			{   
                node1 = new Node();
				node1->id = id1;
				node1->inNodes = new multimap<double, Node *>();
				node1->outNodes = new multimap<double, Node *>();
                nodes->insert(make_pair(id1, node1));
            }else{ // id1����ʧ�ܣ�˵�������ֹ�
                map<int, Node *>::iterator it1 = nodes->find(id1);
                if (it1!= nodes->end()) 
                    node1 = it1->second;
                else 
                    cout << "Find Node" << id1 <<"Error!"<<endl;
            } 

            if (r2.second == true) // id2����ɹ���˵�����δ���ֹ�
			{   
                node2 = new Node();
				node2->id = id2;
				node2->inNodes = new multimap<double, Node *>();
				node2->outNodes = new multimap<double, Node *>();
                nodes->insert(make_pair(id2, node2));
            }else{ // id2����ʧ�ܣ�˵�������ֹ�
                map<int, Node *>::iterator it2 = nodes->find(id2);
                if (it2!= nodes->end()) 
                    node2 = it2->second;
                else 
                     cout << "Find Node" << id2 <<"Error!"<<endl;
            } 
			
			//�������������Ϣ
			node1->outNodes->insert(make_pair(b, node2));
			node2->inNodes->insert(make_pair(b, node1));
        } 
       dataFile.close();

	} else {
        cout << "Cannot open file! File Name is \"" << file <<"\""<<endl;
		exit(1);
    } 

    //����������ȳ��������Լ��㶥�㸲��
	map<int,Node*>::iterator it = nodes->begin();
	while( it != nodes->end() )
	{
		Node *node = it->second;
		node->inDegree = node->inNodes->size();
		node->outDegree = node->outNodes->size();
		it++;
	}

	//�����ʾ��Ϣ
    cout << "NodeIds size: " << nodeIds.size()<< endl;
	cout << "Nodes size: " << nodes->size() << endl;
    cout << "Edges num: " << edges << endl;
	cout << "Read Data Set Complete." << endl;
}

/*ʹ��������̰�Ĺ����ҵ���㸲��*/
void SimPath::findVertexCover(){
	cout << "Calculating Vertex Cover..." << endl;
	
	//���������ȴ�С���������У�mapĬ�ϴ�С����
    multimap<int, Node*> degreeQueue;
	map<int, Node*>::iterator it = nodes->begin();
	while( it != nodes->end() )
	{
        Node *node = it->second;  
		degreeQueue.insert(make_pair(node->inDegree, node));
		it++;
    } 
    
	//����������̰��ԭ�򣬴Ӵ�С˳���ȡ�����μ���VC or NonVC
    multimap<int, Node *>::reverse_iterator rit = degreeQueue.rbegin(); 
	while (vcNodeIds.size() + nvcNodeIds.size() < nodeIds.size()) {
        Node *node = rit->second;
        ++rit;
		int id = node->id;

        //����ǰnode��δ������NonVC�����������VC���ϣ�̰��ԭ��
		if (nvcNodeIds.find(id) == nvcNodeIds.end()) {
			vcNodeIds.insert(id);
        } 
        //������֮���ڵĽ����Ϣ���������ڱ�ȥ��
		multimap<double,Node*> *in = node->inNodes;
		multimap<double,Node*>::iterator itIn = in->begin();
		while( itIn != in->end() )
		{
			Node *inNode = itIn->second;
			inNode->outDegree--;           
			if ( inNode->outDegree == 0 && inNode->inDegree == 0) {
				if ( vcNodeIds.find(inNode->id) == vcNodeIds.end()) {
					nvcNodeIds.insert(inNode->id);
                } 
            } 
			itIn++;
        }
		multimap<double,Node*> *out = node->outNodes;
		multimap<double,Node*>::iterator itOut = out->begin();
		while( itOut != out->end() )
		{
			Node *outNode = itOut->second;
			outNode->inDegree--;          
			if ( outNode->inDegree == 0 && outNode->outDegree == 0) {
				if ( vcNodeIds.find(outNode->id) == vcNodeIds.end()) {
					nvcNodeIds.insert(outNode->id);
                } 
            } 
			itOut++;
        }
	}

	//�����ʾ��Ϣ
	cout << "VC size: " << vcNodeIds.size() << endl;
    cout << "NonVC size: " << nvcNodeIds.size() << endl;
}

/*�㷨���ģ�����Ѱ��k�����ӽ�㱣����seedNodeIds��*/
void SimPath::simPathCore(){
	cout << "Start Calculating..." << endl;

    multimap<double, int> celfQueue; //����marginal gain���򱣴���id
	double totalInf = 0;//������Ӱ��ֵ
	int count = 0;//��¼ÿ�����ӽ����Ҫ����Spread����

    seedNodeIds.clear();

    //��ʼ�����в��ڽ�㸲�Ǽ����еĵ��Ӱ��ֵΪ0
	//map<int,double> nvcInf;
	for(set<int>::iterator it = nvcNodeIds.begin();it!=nvcNodeIds.end();it++){
		nvcInf.insert(make_pair(*it,1.0));
	}
    
    // ���������ڽ�㸲�Ǽ����еĵ��Ӱ��ֵ��ͬʱ����Theorem2���²��ڽ�㸲�Ǽ����еĵ��Ӱ��ֵ
	for(set<int>::iterator it = vcNodeIds.begin();it!=vcNodeIds.end();it++){
		int id = *it;
        double inf = simPathSpreadFirst(id);
        celfQueue.insert(make_pair(inf, id));
		count++;
	}

    // �������в��ڽ�㸲�Ǽ����еĵ��Ӱ��ֵ
	for (map<int, double>::iterator it = nvcInf.begin(); it != nvcInf.end(); it++) {
        int id = it->first;
        double inf = it->second; //����Theorem 2
        celfQueue.insert(make_pair(inf, id));
		count++;
    }
    
	//CELF������MG���ļ�Ϊ��һ��Seed Node
	multimap<double,int>::iterator it = celfQueue.end();
	it--;
	totalInf = it->first;
	seedNodeIds.insert(it->second);
	celfQueue.erase(it);
	cout<<"No "<<seedNodeIds.size()<<" seed: id = "<<*seedNodeIds.begin()<<", mg = "<<totalInf<<", total= "<<totalInf<<", calculate margin gain num ="<<count<<endl;

    // �����ӽ��ѡ����ɣ�����Look Ahead Optimizationԭ����н��������ӽ��ѡȡ
    count = 0;
    set<int> examinedNodeIds; //���ڵ�ǰS���¼���Margin Gain�Ľ��
	set<int> topLNodeIds; // CELF������MG����L�����
    int realTopL;  

    //����S���·�������Margin Gainѡ�����ӽ��
    while (seedNodeIds.size() < k) {
		topLNodeIds.clear();
		if (celfQueue.size() < topL) {
			realTopL = celfQueue.size()-1;
		}else{
			realTopL = topL;
		}
		multimap<double, int>::iterator itCelf = celfQueue.end();
		itCelf--;

		int compareId;
		double compareMg;
		bool findExamined = false;
		bool topOneExamined = false;
        for (int i = 0; i < realTopL; i++) {
            int xId = itCelf->second;
			// ��ǰ���δ�����¼����
            if (examinedNodeIds.find(xId) == examinedNodeIds.end()) {
                topLNodeIds.insert(xId);
				examinedNodeIds.insert(xId);//���������¼��㣡����
                count++;
            } else {
                // ��topL���ҵ�δ�����¼���Ľ��
                findExamined = true;
                compareId = xId;
                compareMg = itCelf->first;
                if (i == 0) {
                    topOneExamined = true; 
                }
                break;
            } 
            itCelf--; 
        }

        // ��ѡ��㼯�ϵ�һ���ͱ����¼��㣬��ֱ�Ӽ������Ӽ�����,������Ӱ��ֵ������CELF����
        if (topOneExamined == true) {
            totalInf += compareMg;
			seedNodeIds.insert(compareId);
			cout<<"No "<<seedNodeIds.size()<<" seed: id = "<<compareId<<", mg = "<<compareMg<<", total= "<<totalInf<<", calculate margin gain num ="<<count<<endl;
            count = 0;
			examinedNodeIds.clear(); //���Ӽ��ϱ仯�����н��������¼���
            celfQueue.erase(itCelf);
            continue;
        }
		// ��Top L�е����н�㶼δ���¼��㣬��Ҫ����һ��L���������бȽ�
        if (findExamined == false) {
            itCelf--;
            compareId = itCelf->second;
            compareMg = itCelf->first;
        }

        // ������TopL��ѡ����CELF������ɾ��
		for (int i = 0; i < topLNodeIds.size(); i++) {
             multimap<double, int>::iterator iter = celfQueue.end();
             iter--;
             celfQueue.erase(iter);
        }
        
		
		int maxId = 0;
		double maxMg = 0;

		//���� inf{V-x}(S)
		infSUp.clear();
		simPathSpreadNormal(seedNodeIds, topLNodeIds); //����infSUp
        
		//���� inf{V-S}(x)����Margin Gain��������������
		infSDown.clear();
		for (set<int>::iterator it = topLNodeIds.begin(); it != topLNodeIds.end(); it++) {
            int xId = *it;
			double xInf = backtrackSimple(xId, seedNodeIds); //����inf{V-S}(x)
            // compute node x's marginal gain;
            double xMg = infSUp.find(xId)->second + xInf - totalInf;
            infSDown.insert(make_pair(xId, xMg)); // pair(ID, newMG)
			if(xMg > maxMg)
			{
				maxId = xId;
				maxMg = xMg;
			}
        }
        if (maxMg >= compareMg) {  // TopL�д��ھ������¼��������MG�Ľ�㣬�������Ӽ�����,������Ӱ��ֵ������CELF����
            totalInf += maxMg;
            seedNodeIds.insert(maxId);
			cout<<"No "<<seedNodeIds.size()<<" seed: id = "<<maxId<<", mg = "<<maxMg<<", total= "<<totalInf<<", calculate margin gain num ="<<count<<endl;
            count = 0;
            examinedNodeIds.clear();

            //��TopL���������н�����²���CELF����
            for (set<int>::iterator it = topLNodeIds.begin(); it != topLNodeIds.end(); it++) {
                int xId = *it;
				if ( xId != maxId) {
                    double xMg = infSDown.find(xId)->second;
                    celfQueue.insert(make_pair(xMg, xId));
                } 
            } 
            continue;
        } else {   // TopL�����н�㶼������һ��L��������MG�������н�����²���CELF����
            for (set<int>::iterator it = topLNodeIds.begin(); it != topLNodeIds.end(); it++) {
                int xId = *it;
                double xMg = infSDown.find(xId)->second;
                celfQueue.insert(make_pair(xMg, xId));
            }

        }
	}
}

/*������Ϊid�Ľ���Ӱ��ֵ��ͬʱ����Theorem2���²��ڽ�㸲�Ǽ����еĵ��Ӱ��ֵ*/
double SimPath::simPathSpreadFirst(int id){

	Node *uNode = nodes->find(id)->second; // ����id�ҵ����u
    uNode->inf = 1;
    uNode->pp = 1;
	uNode->onCurrentPath = true;
	int uId = uNode->id;
	int uOutNum = uNode->outNodes->size();

	//����U��U = {x|x���ڽ�㸲�Ǽ���������u�����㣩
	set<int> UIds;
	map<int, Node *> UNodes;
	map<int, double> bMap; // ��¼u���������Ӱ������ֵ
	multimap<double, Node*> *in = uNode->inNodes; 
	for (multimap<double, Node *>::iterator it = in->begin(); it != in->end(); it++) {
		Node *v = it->second;
		int vId = v->id;
		if (nvcNodeIds.find(vId) != nvcNodeIds.end()) {
			UIds.insert(vId);
			bMap.insert(make_pair(vId, it->first));
			v->inf = 1; //��¼u��V-v�ϵ�Ӱ��ֵ
			v->pp = 1;  //��¼u��v��·������
			v->onCurrentPath = false;
			UNodes.insert(make_pair(vId, v));
        } 
	} 

    double inf = 1; // ��¼u����Ӱ��ֵ
    double pp = 1; // ��¼·������ 
	vector<Node *> Q; //��¼��ǰ·�����
    set<int> idQ; //��¼��ǰ·�����id
    map<int, set<int> > D; //��¼��㱻���ʹ����ھ�

    Q.push_back(uNode);
    idQ.insert(uId);
   
    while (Q.empty() == false) {
        int lastNodeId = 0;
        while (true) {
            // ����Forward��Ѱ��·��
            Node *xNode = Q.back(); // ��ȡ��ǰ·�����һ�����
            int xId = xNode->id; 
            if (xId == lastNodeId) { //��ǰ���·�������������whileѭ������Backtrace
                break;
            }
            lastNodeId = xId;

            pp = xNode->pp; // ��ȡu����ǰ���x��·������

			multimap<double,Node *> *outFromX = xNode->outNodes;
            if (outFromX == NULL || outFromX->empty() == true) {
                continue; //��ǰ���xû�г����������
            }

			//������ǰ���x�����г������Ѱ��·��
            for (multimap<double,Node*>::iterator it = outFromX->begin(); it != outFromX->end(); it++) {
                Node *yNode = it->second; 
                int yId = yNode->id;
           
                if (yId == uId) { // y����uʱ����
                    D[xId].insert(yId);
                    continue;
                } else if (idQ.find(yId) != idQ.end()) { // y ��·�����Ѿ����ڣ�������ֹ�γɻ�
                    continue;
                } else if (D[xId].find(yId) != D[xId].end()) { // y �Ѿ����ʹ���������ֹ�ظ�����
                    continue;
                }

                double ppNext = pp * it->first; // �ҵ��´���·��������·������

                //�ж��Ƿ�ﵽ��ֵ����������ֵ��ֹͣ����·��
                if ( ppNext < pruneVal) {
                    inf += ppNext;
                    D[xId].insert(yId); //��y����x���ѷ����б���
                    continue;
                } 

				//��������ֵ���ص�ǰ������������
                inf += ppNext;  //����u����Ӱ��ֵ
                yNode->pp = ppNext; // ����u�����y��·������ֵ
				yNode->onCurrentPath = true; //����y�ڵ�ǰ·����
                Q.push_back(yNode); // ��y��������ǰ·�����Q��
                idQ.insert(yId); // ��y����ż�����ǰ·�����idQ��
                D[xId].insert(yId); // ���x�����ھ�y�ѱ����ʹ�

                // ����Ӱ��ֵInf{V-v}(u)
                for (map<int, Node *>::iterator it = UNodes.begin(); it != UNodes.end(); ++it) {
					if (it->second->onCurrentPath == false) {
                        it->second->inf += ppNext;
                    }
                }
                break; //������ǰforѭ������y�������һ�����Ѱ��·��
            } 
		}

        //��ǰ·�����һ�������������ϣ�ͨ��Backtrack����Ѱ����·����
        Node *lastNode = Q.back();
		int lastId = lastNode->id;

        if (Q.size() == 1) {   
            if(lastId != uId) {
                cout << "The only remaining node in Q is: " << lastId << ", but not: " << uId << endl;
                exit(1);
            } 
            if(D[lastId].size() < uOutNum) {
                continue;
            } 
        }

        //�����һ�����ӵ�ǰ·��ɾ��
		map<int, Node*>::iterator it = UNodes.find(lastId);
        if (it != UNodes.end()) {
			it->second->onCurrentPath = false;
        }
        idQ.erase(lastId);
        D.erase(lastId);
        Q.pop_back();
    }


    // ����U��Ӱ��ֵ��U = {x|x���ڽ�㸲�Ǽ���������u�����㣩
    for (map<int, Node *>::iterator it = UNodes.begin(); it != UNodes.end(); ++it) {
        int xId = it->first;
        Node *xNode = it->second;
        nvcInf[xId] += bMap[xId] * xNode->inf; //����Theorem2
    } 

    return inf;
}
/*����Ӱ��ֵInf(S)��ͬʱ����Inf{V-x}(S)��x����U*/
double SimPath::simPathSpreadNormal(set<int> S, set<int> U)
{
	double inf = 0;
	infSUp.clear();
	for (set<int>::iterator it = U.begin(); it != U.end(); ++it){
		infSUp.insert(make_pair(*it, 0.0));
    } 
        
    for (set<int>::iterator it = S.begin(); it != S.end(); ++it) {
		int u = *it;
		inf +=  backtrackNormal(u,S,U);
	} 
	return inf;
}

/*����Inf{V-S+u}(u)��ͬʱ����Inf{V-S+u-x}(u)��x����U*/
double SimPath::backtrackNormal(int id, set<int> S,set<int> U){
	Node *uNode = nodes->find(id)->second; // ����id�ҵ����u
	int uId = uNode->id;
	int uOutNum = uNode->outNodes->size();
    uNode->inf = 1;
    uNode->pp = 1;
	uNode->onCurrentPath = true;

	//����U
	set<int> UIds;
	map<int, Node *> UNodes;
	for (set<int>::iterator it = U.begin(); it != U.end(); it++) {
		Node *v = nodes->find(*it)->second;
		int vId = v->id;
		v->inf = 1; //��¼u��V-v�ϵ�Ӱ��ֵ
		v->pp = 1;
		v->onCurrentPath = false;
		UNodes.insert(make_pair(vId, v));
		UIds.insert(vId);
	} 

    double inf = 1; // cov(u), initially 1 (counting itself first)
    double pp = 1; // path prob. 
	vector<Node *> Q; //��¼��ǰ·�����
    set<int> idQ; //��¼��ǰ·�����id
    map<int, set<int> > D; //��¼��㱻���ʹ����ھ�

    Q.push_back(uNode);
    idQ.insert(id);
   
    while (Q.empty() == false) {
        int lastNodeId = 0;
        while (true) {
            // FORWARD starts here!!!
            Node *xNode = Q.back(); // ��ȡ��ǰ·�����һ�����
            int xId = xNode->id; 
    
            if (xId == lastNodeId) {
                break;
            }
            lastNodeId = xId;

            pp = xNode->pp; // get the current path prob. till this node

			multimap<double,Node *> *outFromX = xNode->outNodes;
            if (outFromX == NULL || outFromX->empty() == true) {
                continue; //��ǰ���xû���ھ�
            }

            for (multimap<double,Node*>::iterator it = outFromX->begin(); it != outFromX->end(); it++) {
                Node *yNode = it->second; 
                int yId = yNode->id;
           
                if (yId == uId) { // y����uʱ
                    D[xId].insert(yId);
                    continue;
                } else if (idQ.find(yId) != idQ.end()) { // y ��·�����Ѿ����ڣ�������ֹ�γɻ�
                    continue;
                } else if (D[xId].find(yId) != D[xId].end()) { // y �Ѿ��������������ֹ�ظ�����
                    continue;
				} else if (S.find(yId)!=S.end()){  //y����W��V-S����
					continue;
				}

                double ppNext = pp * it->first; // pp = pp * b(x,y)
                // �ж��Ƿ�ﵽ��ֵ
                if ( ppNext < pruneVal) {
                    inf += ppNext;
                    D[xId].insert(yId); // y is explored
                    continue;
                } 

				//��������ֵδ��������˵���ҵ�һ����·��
                inf += ppNext;  //����u����Ӱ��ֵ
                yNode->pp = ppNext; // ����u�����y��·������ֵ
				yNode->onCurrentPath = true; //��ʱy�ڵ�ǰ·���� -->�ɸ�

                Q.push_back(yNode); // ��y��������ǰ·�����Q��
                idQ.insert(yId); // ��y���id������ǰ·�����idQ��
                D[xId].insert(yId); // ��¼x�����ھ�y�ѱ����ʹ�

                // ����Ӱ��ֵ��V-v��
                for (map<int, Node *>::iterator it = UNodes.begin(); it != UNodes.end(); ++it) {
					if (it->second->onCurrentPath == false) {
                        it->second->inf += ppNext;
                    }
                }
                break;
            } 

		}

         //��ǰ·�����һ�������������ϣ�ͨ��Backtrack����Ѱ����·����
        Node *lastNode = Q.back();
		int lastId = lastNode->id;

        if (Q.size() == 1) {   
            if(lastId != uId) {
                cout << "The only remaining node in Q is: " << lastId << ", but not: " << uId << endl;
                exit(1);
            } 
            if(D[lastId].size() < uOutNum) {
                continue;
            } 
        }

        //�����һ�����ӵ�ǰ·��ɾ��
		map<int, Node*>::iterator it = UNodes.find(lastId);
        if (it != UNodes.end()) {
			it->second->onCurrentPath = false;
        }
        idQ.erase(lastId);
        D.erase(lastId);
        Q.pop_back();

    }


    // update partial coverage for U 
    for (map<int, Node *>::iterator it = UNodes.begin(); it != UNodes.end(); ++it) {
        int xId = it->first;
        Node *xNode = it->second;
		infSUp[xId] += xNode->inf; 
    } 
    return inf;
}

/*���ݱ��id����Inf{V-S}(x)��ֵ*/
double SimPath::backtrackSimple(int id, set<int> S){
	Node *uNode = nodes->find(id)->second; // ����id�ҵ����u
	int uId = uNode->id;
    uNode->inf = 1;
    uNode->pp = 1;

    double inf = 1; // cov(u), initially 1 (counting itself first)
    double pp = 1; // path prob. 
	vector<Node *> Q; //��¼��ǰ·�����
    set<int> idQ; //��¼��ǰ·�����id
    map<int, set<int> > D; //��¼��㱻���ʹ����ھ�

    Q.push_back(uNode);
    idQ.insert(id);
   
    while (Q.empty() == false) {
        int lastNodeId = 0;
        while (true) {
            // FORWARD starts here!!!
            Node *xNode = Q.back(); // ��ȡ��ǰ·�����һ�����
            int xId = xNode->id; 
    
            if (xId == lastNodeId) {
                break;
            }
            lastNodeId = xId;

            pp = xNode->pp; // get the current path prob. till this node

			multimap<double,Node *> *outFromX = xNode->outNodes;
            if (outFromX == NULL || outFromX->empty() == true) {
                continue; //��ǰ���xû���ھ�
            }

            for (multimap<double,Node*>::iterator it = outFromX->begin(); it != outFromX->end(); it++) {
                Node *yNode = it->second; 
                int yId = yNode->id;
           
                if (yId == uId) { // y����uʱ
                    D[xId].insert(yId);
                    continue;
                } else if (idQ.find(yId) != idQ.end()) { // y ��·�����Ѿ����ڣ�������ֹ�γɻ�
                    continue;
                } else if (D[xId].find(yId) != D[xId].end()) { // y �Ѿ��������������ֹ�ظ�����
                    continue;
				} else if (S.find(yId)!=S.end()){  //y����W��V-S����
					D[xId].insert(yId);
					continue;
				}

                double ppNext = pp * it->first; // pp = pp * b(x,y)
                // �ж��Ƿ�ﵽ��ֵ
                if ( ppNext < pruneVal) {
                    inf += ppNext;
                    D[xId].insert(yId); // y is explored
                    continue;
                } 

				//��������ֵδ��������˵���ҵ�һ����·��
                inf += ppNext;  //����u����Ӱ��ֵ
                yNode->pp = ppNext; // ����u�����y��·������ֵ
				yNode->onCurrentPath = true; //��ʱy�ڵ�ǰ·���� -->�ɸ�

                Q.push_back(yNode); // ��y��������ǰ·�����Q��
                idQ.insert(yId); // ��y���id������ǰ·�����idQ��
                D[xId].insert(yId); // ��¼x�����ھ�y�ѱ����ʹ�

                break;
            } 

		}

        //����Backtrace����Ѱ����һ����̽��·���ĵ�
        Node *lastNode = Q.back(); 
		int lastId = lastNode->id;
        idQ.erase(lastId);
        D.erase(lastId);
        Q.pop_back();
    }

    return inf;
}

int main()
{
	SimPath *s = new SimPath();
	double _pruneVal = 0.001; 
	int _topL = 4;
	int _k = 5;
	string file = "hep_LT2.inf";
	s->setParameter(_pruneVal,_topL,_k);
	s->readDataSets(file);
	s->findVertexCover();
	s->simPathCore();
	return 0;
}