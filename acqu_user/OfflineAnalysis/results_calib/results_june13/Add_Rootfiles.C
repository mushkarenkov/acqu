#include <string.h>
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

TList *FileList;
TFile *Target;

ifstream file;
char item[100];
char previous[100];

void MergeRootfile( TDirectory *target, TList *sourcelist );

// void Add_Rootfiles(char list[100]) {
void Add_Rootfiles() {

  FileList = new TList();
 
  sprintf(previous,"nothing");

//   TString list = "list_add.txt";
  TString list = "add_may.txt";
  file.open(list); 
  
  int nlines = 0;
  
  if (file.is_open()){
    cout << "\n list open!" << endl;
    while (!file.eof()){
      if (!file.good()) break;
      file >> item;
      if (strcmp(previous,item) == 0) break;
      cout << "line " << nlines << ",  rootfile to merge is " << item << endl;
      FileList->Add( TFile::Open(item) );
      nlines++;
      sprintf(previous,"%s",item);
    }
    file.close(); 
  }
  
  else {
    cout << "list cannot be opened, sorry for the headache :(" << endl;
  }


  Target = TFile::Open( "nsummed.root", "RECREATE" );


  MergeRootfile( Target, FileList );
   
}   

void MergeRootfile( TDirectory *target, TList *sourcelist ) {
  
  cout << "Target path: " << target->GetPath() << endl;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    // read object from first source file
    first_source->cd( path );
    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
      // descendant of TH1 -> merge it

      cout << "Merging histogram " << obj->GetName() << endl;
      TH1 *h1 = (TH1*)obj;
      h1->Sumw2();

      // loop over all source files and add the content of the
      // correspondant histogram to the one pointed to by "h1"
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      while ( nextsource ) {
        
        // make sure we are at the correct directory level by cd'ing to path
        nextsource->cd( path );
        TH1 *h2 = (TH1*)gDirectory->Get( h1->GetName() );
        if ( h2 ) {
	  h2->Sumw2();
          h1->Add( h2 );
          delete h2; // don't know if this is necessary, i.e. if 
                     // h2 is created by the call to gDirectory above.
        }

        nextsource = (TFile*)sourcelist->After( nextsource );
      }
    }
    else if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
      
      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();
      
      cout << "Adding tree " << obj_name << endl;

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
//      const char* file_name = nextsource->GetName();
//      cout << "file name  " << file_name << endl;
     while ( nextsource ) {
     	  
       globChain->Add(nextsource->GetName());
       nextsource = (TFile*)sourcelist->After( nextsource );
     }

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, sourcelist );

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: " 
           << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();

      //!!if the object is a tree, it is stored in globChain...
	if(obj->IsA()->InheritsFrom( "TTree" ))
	  globChain->Write( key->GetName() );
	else
	obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->Write();

}
