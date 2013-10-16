/////////////////////////////////////
// Author: S.Costanza -  June 2013 //
/////////////////////////////////////

const Int_t fNch = 2;
ofstream fout;

void Write_StartingPar() {

  TString input_pars = "mwpc_params.dat";
  fout.open(input_pars, ios::out);

  for (int iCh = 0; iCh < fNch; iCh++) {
    for (int i = 0; i < 14; i++) {
      fout << "0\t" ;
      if (i==13)
	fout << endl;
    }
  }
  fout.close();
}
