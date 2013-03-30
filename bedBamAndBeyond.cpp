/*
 * bedBamAndBeyond
 * Date: Mar-29-2013 
 * Author : Gabriel Renaud gabriel.reno [at sign here ] gmail.com
 *
 */

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <gzstream.h>
#include <setjmp.h>

#include "api/BamMultiReader.h"
#include "api/BamReader.h"
#include "api/BamWriter.h"
#include "api/BamAux.h"

#include "hpdf.h"
#include "utils.h"

using namespace std;
using namespace BamTools;

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}



void
draw_rect (HPDF_Page     page,
           double        x,
           double        y,
	   double length,
           const char   *label){    
    HPDF_Page_BeginText (page);
    //text pos
    HPDF_Page_MoveTextPos (page, x, y - 10);
    HPDF_Page_ShowText (page, label);
    HPDF_Page_EndText (page);
    //rectangle
    HPDF_Page_Rectangle(page, x, y - 30, length, 15);
}


void
draw_Simplerect (HPDF_Page     page,
		 double        x,
		 double        y,
		 double length){    
    // cout<<"x "<<x<<endl;
    // cout<<"y "<<y<<endl;

    length=max(length,0.4);
    // cout<<"l "<<length<<endl;
    // HPDF_Page_BeginText (page);
    // //text pos
    // HPDF_Page_MoveTextPos (page, x, y - 10);
    // HPDF_Page_ShowText (page, label);
    // HPDF_Page_EndText (page);
    //rectangle

    HPDF_Page_Rectangle(page, x, y - 30, length, 15);
}

typedef struct{
    string name;
    unsigned int startIndexChr;
    unsigned int endIndexChr;
    unsigned int length;
} chrinfo;

typedef struct{
    double  y;
    double  length;
    double  lengthScreen;
} chrScreenInfo;

inline void addRange(HPDF_Page & page,double begin,double end, const chrScreenInfo & chrInfToUse ){
    draw_Simplerect (page, 
		     10+ chrInfToUse.lengthScreen*(begin/chrInfToUse.length ),
		     chrInfToUse.y,
		     chrInfToUse.lengthScreen * ((end-begin)/chrInfToUse.length) );			     
    // cout<<line<<endl;
    HPDF_Page_Fill (page);
}

bool isBAM(string filename){
    bool isBAMfile=true;
    //HPDF_Page_SetAlphaFill (page, 0.75);
    igzstream testFILE;

    testFILE.open(filename.c_str(), ios::in);
    if (testFILE.good()){
	const string magicNumber="BAM\1";
	unsigned char  stringHeader [4];
	const unsigned int     lengthHeader  = 4;

	testFILE.read((char*)&stringHeader,lengthHeader);

	for(unsigned int i=0;i<lengthHeader;i++){
	    if(magicNumber[i] != stringHeader[i]){
		isBAMfile=false;
	    }
	}       
	testFILE.close();
    }else{
	cerr << "Unable to open file "<<filename<<endl;
	exit(1);
    }
    return isBAMfile;
}

int main (int argc, char *argv[]) {

    string faidx="/mnt/454/Altaiensis/users/gabriel/faidx/index.hg19.fai";
    double alpha=0.8;
    const string usage=string(string(argv[0])+" [options]  <in BED/BAM> <out pdf>"+"\n\n"+
			      "this program create an ideogram [out pdf] with the [in BED] file\n"+
			      "\n"+
			      "Options:\n"+
			      "\t"+"--fai" +"\t\t\t"+"samtools faidx for the genome (Default : "+faidx+"\n"+
			      "\t"+"--alpha" +"\t\t\t"+"Alpha parameter for bed chunks (Default : "+stringify(alpha)+"\n");
			      

     if( (argc== 1) ||
    	(argc== 2 && string(argv[1]) == "-h") ||
    	(argc== 2 && string(argv[1]) == "-help") ||
    	(argc== 2 && string(argv[1]) == "--help") ){
	 cerr<<usage<<endl;
	 return 1;
    }

    for(int i=1;i<(argc-2);i++){ //all but the last two arg

    	if( (string(argv[i]) == "--faidx") ){
    	    faidx=string(argv[i+1]);
	    i++;
    	    continue;
    	}

    	if( (string(argv[i]) == "--alpha")  ){
    	    alpha=destringify<double>(argv[i+1]);	    
	    i++;
    	    continue;
    	}

       
    	cerr<<"Unknown option "<<argv[i] <<" exiting"<<endl;
    	return 1;
    }

    if(alpha<0.0 ||  alpha>1.0){
    	cerr<<"alpha must be between 0 and 1, exiting"<<endl;
    	return 1;

    }


    string bedFile    = string(argv[argc-2]);
    string fname      = string(argv[argc-1]);
    string page_title = "";

    HPDF_Doc  pdf;
    HPDF_Font font;
    HPDF_Page page;
    HPDF_ExtGState gstate;
    float tw;
 

    pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return 1;
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }
    font = HPDF_GetFont (pdf, "Helvetica", NULL);

    /* add a new page object. */
    page = HPDF_AddPage (pdf);





    // /* print the lines of the page. */
    //HPDF_Page_SetLineWidth (page, 1);
    // HPDF_Page_Rectangle (page, 50, 50, HPDF_Page_GetWidth(page) - 100,
    //             HPDF_Page_GetHeight (page) - 110);
    // HPDF_Page_Stroke (page);

    /* print the title of the page (with positioning center). */
    HPDF_Page_SetFontAndSize (page, font, 24);
    tw = HPDF_Page_TextWidth (page, page_title.c_str());
    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, (HPDF_Page_GetWidth(page) - tw) / 2,
			   HPDF_Page_GetHeight (page) - 50);
    //HPDF_Page_ShowText (page, page_title.c_str());
    HPDF_Page_EndText (page);
    HPDF_Page_SetFontAndSize (page, font, 10);
    

  

    // cout<<HPDF_Page_GetWidth(page)<<endl;
    // cout<<HPDF_Page_GetHeight(page)<<endl;

    string line;
    igzstream myFaidxFile;

    myFaidxFile.open(faidx.c_str(), ios::in);


    vector<chrinfo> chrFound;
    unsigned int  genomeLength=0;
    unsigned int  maxLengthFound=0;
    if (myFaidxFile.good()){
	while ( getline (myFaidxFile,line)){
	    chrinfo toadd;
	    vector<string> fields = allTokens(line,'\t');

	    toadd.name         =fields[0];
	    toadd.startIndexChr=genomeLength+1;
	    toadd.length       =destringify<unsigned int>(fields[1]);
	    if(toadd.length> maxLengthFound){
		maxLengthFound = toadd.length;
	    }
	    toadd.endIndexChr  =genomeLength+toadd.length;
	    chrFound.push_back(toadd);

	}
	myFaidxFile.close();
    }else{
	cerr << "Unable to open file "<<faidx<<endl;
	return 1;
    }

    double sizeToUse=HPDF_Page_GetHeight(page)/double(2.0*chrFound.size());

    map<string, chrScreenInfo>  name2chrScreenInfo;
    // map<string, double>  name2y;
    // map<string, double > name2length;
    // map<string, double > name2lengthScreen;
    // map<string, double > name2WidthScreen;

    double widthScreen= (HPDF_Page_GetWidth(page)-10.0);
    for(unsigned int i=0;
	i<chrFound.size();
	i++){
	//cout<<<<endl;
	  //                x  y   title
	//draw_rect (page, HPDF_Page_GetWidth(page) -10.0*double(i) , 100, chrFound[i].name.c_str());
	// draw_Simplerect (page, 
	// 		 10 , //x
	// 		 HPDF_Page_GetHeight(page) -sizeToUse*double(i*2), //y
	// 		 (HPDF_Page_GetWidth(page)-10.0)  * (double(chrFound[i].length)/double(maxLengthFound)));    //length

	draw_rect (page, 
		   10 , //x
		   HPDF_Page_GetHeight(page) -sizeToUse*double(i*2), //y
		   widthScreen  * (double(chrFound[i].length)/double(maxLengthFound)),     //length
		   chrFound[i].name.c_str());

	name2chrScreenInfo[      chrFound[i].name.c_str() ].y       = HPDF_Page_GetHeight(page) -sizeToUse*double(i*2);
	name2chrScreenInfo[ chrFound[i].name.c_str() ].length       = double(chrFound[i].length);

	name2chrScreenInfo[ chrFound[i].name.c_str() ].lengthScreen = ( (HPDF_Page_GetWidth(page)-10.0)  * (double(chrFound[i].length)/double(maxLengthFound)) );     //length
	// cout<<chrFound[i].name.c_str()  <<"\t"<<name2lengthScreen[ chrFound[i].name.c_str() ]<<endl;
	//	name2WidthScreen[ chrFound[i].name.c_str() ] =   widthScreen  * (double(chrFound[i].length)/double(maxLengthFound));
	HPDF_Page_Stroke (page);

    }

    HPDF_Page_GSave (page);
    gstate = HPDF_CreateExtGState (pdf);
    HPDF_ExtGState_SetAlphaFill (gstate, alpha);
    HPDF_Page_SetExtGState (page, gstate);

    HPDF_Page_SetRGBStroke (page, 0.75, 0, 0);
    HPDF_Page_SetRGBFill (page, 0.75, 0.0, 0.0);



    bool isBAMfile=isBAM(bedFile);

    igzstream myBEDFile;

    if(!isBAMfile){
	myBEDFile.open(bedFile.c_str(), ios::in);


	if (myBEDFile.good()){
	    while ( getline (myBEDFile,line)){
		//cout<<line<<endl;
		chrinfo toadd;
		vector<string> fields = allTokens(line,'\t');
		double begin=destringify<double>(fields[1]);
		double end  =destringify<double>(fields[2]);
		if(name2chrScreenInfo.find( fields[0] ) != name2chrScreenInfo.end())
		addRange(page,begin,end,name2chrScreenInfo[fields[0]] );
	    }
	    myBEDFile.close();
	}else{
	    cerr << "Unable to open file "<<bedFile<<endl;
	    return 1;
	}
    }else{
	BamReader reader;
	if ( !reader.Open(bedFile) ) {
	    cerr << "Could not open input BAM files." << endl;
	    return 1;
	}
	vector<RefData>  refData=reader.GetReferenceData();


	BamAlignment al;

	while ( reader.GetNextAlignment(al) ) {
	    // cout<<al.Name<<endl;
	    if(!al.IsMapped())
		continue;	   
	    // cout<<refData[al.RefID].RefName<<"\t"<<double(al.Position)<<endl;
	    if(name2chrScreenInfo.find( refData[al.RefID].RefName ) != name2chrScreenInfo.end())
		addRange(page,
			 double(al.Position),
			 double(al.Position+al.AlignedBases.size()),
			 name2chrScreenInfo[ refData[al.RefID].RefName ] );

	} //while al
	
	reader.Close();
	
    }


    /* save the document to a file */
    HPDF_SaveToFile (pdf, fname.c_str());

    /* clean up */
    HPDF_Free (pdf);

   
    return 0;
}

