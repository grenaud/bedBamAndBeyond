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


//position on the screen
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

inline void addRangeCov(HPDF_Page & page,double begin,double end, const chrScreenInfo & chrInfToUse, double covFrac ){
    // cout<<"1 "<<(covFrac)<<endl;
    // cout<<"2 " <<(1.0-covFrac)<<endl;
    //    covFrac=double(rand())/double(RAND_MAX);
    HPDF_Page_SetRGBStroke (page, 1.00, 1.0-covFrac,  1.0-covFrac);
    HPDF_Page_SetRGBFill   (page, 1.00, 1.0-covFrac,  1.0-covFrac);

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
    int genomicWindow=1000000;
    double topCoverage  =50.0;
    bool coverage=false;
    bool oneChr=false;
    string oneChrName="";
    bool userSetBED=false;
    bool userSetBAM=false;

    const string usage=string(string(argv[0])+" [options] <out pdf>  <in BED/BAM red> [in BED/BAM green] [in BED/BAM blue] "+"\n\n"+
			      "this program create an ideogram [out pdf] with the [in BED] file\n"+
			      "If you used automatic format detection, (not specifying --bed or --bam), do not use file descriptors\n"+
			      "For the bed file, you can enter a weight for that range as a 4th field\n"+
			      "\n"+
			      "Options:\n"+
			      "\t"+"--bed" +"\t\t\t"+"Disable automatic format detection, input is bed (Default: "+booleanAsString(userSetBED)+")\n"+		
			      "\t"+"--bam" +"\t\t\t"+"Disable automatic format detection, input is BAM (Default: "+booleanAsString(userSetBAM)+")\n"+		

			      "\t"+"--cov" +"\t\t\t"+"Plot coverage up to "+stringify(topCoverage)+" with windows "+stringify(genomicWindow)+"\n"+		
			      "\tOptions for this mode:\n"+
			      "\t\t"+"--win" +"\t[size]\t\t"+"Use this size instead of "+stringify(genomicWindow)+"\n"+
			      "\t\t"+"--top" +"\t[coverage]\t"+"Use this as top coverage instead of "+stringify(topCoverage)+"\n"+
			      
			      "\t"+"--fai" +"\t[fai file]\t\t"+"samtools faidx for the genome (Default : "+faidx+")\n"+
			      "\t"+"--chr" +"\t[chr name]\t\t"+"Just plot this chromsome, must be in the fai file (Default : all)\n"+

			      "\t"+"--alpha" +"\t[alpha]\t\t"+"Alpha parameter for bed chunks (Default : "+stringify(alpha)+")\n");
			      

     if( (argc== 1) ||
    	(argc== 2 && string(argv[1]) == "-h") ||
    	(argc== 2 && string(argv[1]) == "-help") ||
    	(argc== 2 && string(argv[1]) == "--help") ){
	 cerr<<usage<<endl;
	 return 1;
     }

     int indexOflastOpt=0;
     for(int i=1;i<(argc);i++){ //all but the last two arg

	 if( string(argv[i]).substr(0,2) != "--" ){
	     indexOflastOpt=i;
	     break;
	 }
	
	 if( (string(argv[i]) == "--chr") ){
	     oneChr    = true;
	     oneChrName= string(argv[i+1]);
	     i++;
	     continue;
	 }

	 if( (string(argv[i]) == "--cov") ){
	     coverage=true;
	     continue;
	 }

	 if( (string(argv[i]) == "--bed") ){
	     userSetBED=true;
	     continue;
	 }

	 if( (string(argv[i]) == "--bam") ){
	     userSetBAM=true;
	     continue;
	 }

	 if( (string(argv[i]) == "--win") ){
	     genomicWindow=destringify<int>(argv[i+1]);
	     i++;
	     continue;
	 }

	 if( (string(argv[i]) == "--top") ){
	     topCoverage=destringify<double>(argv[i+1]);
	     i++;
	     continue;
	 }

	 if( (string(argv[i]) == "--fai") ){
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

     if(userSetBAM && userSetBED){
	 cerr<<"Cannot set both --bed and --bam, exiting"<<endl;
	 return 1;
     }



     string fname      = string(argv[indexOflastOpt]);
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


     bool found=false;
     if(oneChr){
	 for(unsigned int i=0;
	     i<chrFound.size();
	     i++){
	     if(chrFound[i].name == oneChrName){
		 found=true;
		 maxLengthFound=chrFound[i].length;
	     }
	 }

	 if(!found){
	     cerr<<"Chromosome you entered "<<oneChrName<<" was not found"<<endl;
	     return 1;
	 }
     }

     double sizeToUse=HPDF_Page_GetHeight(page)/double(2.0*chrFound.size());

     map<string, chrScreenInfo>  name2chrScreenInfo;

     double widthScreen= (HPDF_Page_GetWidth(page)-10.0);    
     for(unsigned int i=0;
	 i<chrFound.size();
	 i++){

	 double yOffset=sizeToUse*double(i*2);
	 if(oneChr){
	     if(chrFound[i].name != oneChrName)
		 continue;
	     yOffset=0;
	 }
	
	 draw_rect (page, 
		    10 , //x
		    HPDF_Page_GetHeight(page) - yOffset, //y
		    widthScreen  * (double(chrFound[i].length)/double(maxLengthFound)),     //length
		    chrFound[i].name.c_str());

	 name2chrScreenInfo[ chrFound[i].name.c_str() ].y            = HPDF_Page_GetHeight(page) -yOffset; //y offset
	 name2chrScreenInfo[ chrFound[i].name.c_str() ].length       = double(chrFound[i].length);   //length

	 name2chrScreenInfo[ chrFound[i].name.c_str() ].lengthScreen = ( (HPDF_Page_GetWidth(page)-10.0)  * (double(chrFound[i].length)/double(maxLengthFound)) );     //length on screen
	 HPDF_Page_Stroke (page);
     }

     HPDF_Page_GSave (page);
     gstate = HPDF_CreateExtGState (pdf);
     HPDF_ExtGState_SetAlphaFill (gstate, alpha);
     HPDF_Page_SetExtGState (page, gstate);



     for(int indexofinputF=1;(indexOflastOpt+indexofinputF)<argc;indexofinputF++){
	 string bedFile    = string(argv[indexOflastOpt+indexofinputF]);

	 if(indexofinputF==1){
	     HPDF_Page_SetRGBStroke (page, 0.75,  0.0,    0.0);
	     HPDF_Page_SetRGBFill   (page, 0.75,  0.0,    0.0);
	 }else if(indexofinputF==2){
	     HPDF_Page_SetRGBStroke (page, 0.0,  0.75,    0.0);
	     HPDF_Page_SetRGBFill   (page, 0.0,  0.75,    0.0);
	 }else if(indexofinputF==3){
	     HPDF_Page_SetRGBStroke (page, 0.0,   0.0,  0.75 );
	     HPDF_Page_SetRGBFill   (page, 0.0,   0.0,  0.75 );
	 }else{
	     cerr<<"Color not defined for this file"<<endl;
	     return 1;
	 }


	 bool isBAMfile=false;
	 if(userSetBED || userSetBAM){
	     isBAMfile=userSetBAM;
	 }else{
	     isBAMfile=isBAM(bedFile);
	     if(coverage && !isBAMfile){
		 cerr<<"Cannot specify coverage and not a BAM file as input"<<endl;
		 return 1;
	     }
	 }

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
		
		     if(name2chrScreenInfo.find( fields[0] ) != name2chrScreenInfo.end()){
			 if(fields.size() == 4){
			     double factor  =destringify<double>(fields[3]);
			     if(factor>1 ||  factor<0){
				 cerr<<"ERROR: alpha factor must be between 0 and 1"<<factor<<endl;
				 return 1;
			     }
			     addRangeCov(page,
					 begin,
					 end,
					 name2chrScreenInfo[fields[0]],
					 factor);
			 }else{
			     addRange(page,begin,end,name2chrScreenInfo[fields[0]] );
			 }

		     }
		 }
		 myBEDFile.close();
	     }else{
		 cerr << "Unable to open file "<<bedFile<<endl;
		 return 1;
	     }
	 }else{
	     if(!coverage){
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
	     }else{
		 BamReader reader;
		 if ( !reader.Open(bedFile) ) {
		     cerr << "Could not open input BAM files." << endl;
		     return 1;
		 }
		 vector<RefData>  refData=reader.GetReferenceData();
		 if ( !reader.LocateIndex()  ) {
		     cerr << "The index for the BAM file cannot be located" << endl;
		     return 1;
		 }
	    
		 if ( !reader.HasIndex()  ) {
		     cerr << "The BAM file has not been indexed." << endl;
		     return 1;
		 }


		 for( map<string ,chrScreenInfo>::iterator it = name2chrScreenInfo.begin(); it != name2chrScreenInfo.end(); ++it) {
		     //if one chr, skip others
		     if(oneChr){
			 if(it->first != oneChrName)
			     continue;
		     }

		     cerr << "Processing chr : "<<it->first << "\n";
		     int refid=reader.GetReferenceID(it->first);
		     chrScreenInfo tempCInf= name2chrScreenInfo[ it->first ];

		     for(unsigned int coordinate=1;
			 (coordinate+genomicWindow)<it->second.length;
			 coordinate+=genomicWindow){

			 reader.SetRegion(refid,
					  coordinate,
					  refid,
					  coordinate+genomicWindow); 
		    
			 BamAlignment al;
			 unsigned int totalBases=0;
			 while ( reader.GetNextAlignment(al) ) {
			     //cout<<al.Name<<endl;
			     if(!al.IsMapped())
				 continue;	   
			     // cout<<refData[al.RefID].RefName<<"\t"<<double(al.Position)<<endl;
			     totalBases+=al.AlignedBases.size();			
			 } //while al
			 //cout<<double(totalBases)/double(genomicWindow)<<endl;
			 double cover=min(topCoverage,double(totalBases)/double(genomicWindow));
			 //cout<<"c1 "<<cover<<endl;
			 cover=cover/topCoverage;
			 //cout<<"c2 "<<cover<<endl;
			 addRangeCov(page,
				     double(coordinate),
				     double(coordinate+genomicWindow),
				     tempCInf,
				     cover);

		    
		     }

		 }


	    
	     }	
	 }
     }

     /* save the document to a file */
     HPDF_SaveToFile (pdf, fname.c_str());

     /* clean up */
     HPDF_Free (pdf);

   
    return 0;
}

