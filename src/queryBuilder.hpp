#include <bits/stdc++.h>

#define vs vector<string>

using namespace std;
extern unordered_map<string, string> umap;

string decToBinary(int n, int size)
{
    // n --> decimal number to convert
    // size --> number of bits needed in the binary representation

    string binaryStr = "";

    for (int i = size-1; i >= 0; i--) {
        int k = n >> i;
        if (k & 1)
            binaryStr += "1";
        else
            binaryStr += "0";
    }

    return binaryStr;
}

string binaryToHex(string binaryStr){
    string hexStr = "";
    for(int i = 0; i <=binaryStr.length()-4; i+=4){
        hexStr += umap[binaryStr.substr(i,4)];
    }
    return hexStr;
}

string convertDomainNameToBinary(string domainName){
    vs domainNamePartitions;

    char str[domainName.length() + 1]; 
    strcpy(str, domainName.c_str()); 
 
    char *token = strtok(str, ".");

    while (token != NULL)
    {
        domainNamePartitions.push_back(token);
        token = strtok(NULL, ".");
    }

    string binaryStr = "";

    for(int i=0; i<domainNamePartitions.size(); i++){

        string pStr = domainNamePartitions.at(i);

        binaryStr += decToBinary(pStr.length(), 8);

        for(int j=0; j<pStr.length(); j++){
            binaryStr += decToBinary((int)pStr[j], 8);
        }
    }

    binaryStr += decToBinary(0, 8);

    return binaryStr;
 
}

class Header{
    public:

    int ID;
    int OPCODE;
    string QR;
    string AA;
    string TC;
    string RD;
    string RA;
    string Z;
    int RCODE;
    int QDCOUNT;
    int ANCOUNT;
    int NSCOUNT;
    int ARCOUNT;

    Header() {

    }

    Header(int ID, string QR, int OPCODE, string AA, string TC, string RD, string RA, string Z, int RCODE, int QDCOUNT, int ANCOUNT, int NSCOUNT, int ARCOUNT){
        this->ID = ID;
        this->QR = QR;
        this->OPCODE = OPCODE;
        this->AA = AA;
        this->TC = TC;
        this->RD = RD;
        this->RA = RA;
        this->Z = Z;
        this->RCODE = RCODE;
        this->QDCOUNT = QDCOUNT;
        this->ANCOUNT = ANCOUNT;
        this->NSCOUNT = NSCOUNT;
        this->ARCOUNT = ARCOUNT;
    }

    string getBinaryRepresentation(){
        string binaryStr =  decToBinary(this->ID, 16) + 
                            this->QR +
                            decToBinary(this->OPCODE, 4) + 
                            this->AA + 
                            this->TC + 
                            this->RD + 
                            this->RA + 
                            this->Z +
                            decToBinary(this->RCODE, 4) + 
                            decToBinary(this->QDCOUNT, 16) +
                            decToBinary(this->ANCOUNT, 16) +
                            decToBinary(this->NSCOUNT, 16) +
                            decToBinary(this->ARCOUNT, 16);

        return binaryStr;
    }

    string getHexRepresentation(){
        return binaryToHex(this->getBinaryRepresentation());
    }

};

class Question{
    public:

    string QNAME;
    string QTYPE;
    string QCLASS;

    Question(){

    }

    Question(string domainName, int queryType){
        this->QNAME = convertDomainNameToBinary(domainName);
        this->QTYPE = decToBinary(queryType, 16); // for Type A, will try 255 also
        this->QCLASS = decToBinary(1, 16); // for IN class
    }

    string getBinaryRepresentation(){
        string binaryStr = this->QNAME + this->QTYPE + this->QCLASS;
        return binaryStr;
    }

    string getHexRepresentation(){
        return binaryToHex(this->getBinaryRepresentation());
    }

    
};

class DNSQuery {
    public:
    
    int id;
    int opcode;
    int responseCode;
    int qdCount;
    int anCount;
    int nsCount;
    int arCount;
    string domainName;
    int queryType;
    int headerLength;
    int questionLength;

    DNSQuery(int opcode, int responseCode, int qdCount, int anCount, int nsCount, int arCount, string domainName, int queryType){
        
        this->id = rand() % 65536;
        this->opcode = opcode;
        this->responseCode = responseCode;
        this->qdCount = qdCount;
        this->anCount = anCount;
        this->nsCount = nsCount;
        this->arCount = arCount;
        this->domainName = domainName;
        this->queryType = queryType;

    }

    string getDNSQuery(){
        
        Header header(this->id, "0", this->opcode, "0", "0", "1", "0", "000", this->responseCode, this->qdCount, this->anCount, this->nsCount, this->arCount);
        Question question(this->domainName, this->queryType);

        string headerHexString = header.getHexRepresentation();
        this->headerLength = headerHexString.length();

        string questionHexString = question.getHexRepresentation();
        this->questionLength = questionHexString.length();

        string dnsQueryHexString = headerHexString + questionHexString;
        return dnsQueryHexString;
    }
};