/* bhc.c ...Binary data <-> Intel Hex data Converter Ver 1.0.3 */
/* Suport ROM size 16k,32k,64k,128k,256k,512k,1024k */
/* 1998/9/12  Kazuhiko Mori(COW) (C)Cow Project Workshop */
/* e-mail:cow@he.mirai.ne.jp */
/* This Program is Free Soft */
/* This version for Unix,Win32,OS/2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

/* function prototype */
int b2h(char *in,char *out, char *bufs);
int h2b(char *in,char *out, char *bufs);
int scnv(char *size_ch);
int ch2num(char ch1,char ch2);

int main(int argc,char *argv[])/*Main routine*/
{
  int i;
  char inf[255],outf[255],flg[8],bufs[8];

  i = 0;/*Set return code*/
  printf("bhc Ver1.0.3 Binary data <-> Intel Hex data Converter.\n");
  printf("(C)1997-1998 Cow Project Workshop.\n");
  if(argc == 5){ /* Parameter check*/
    strcpy(flg,argv[1]);
    strcpy(inf,argv[2]);
    strcpy(outf,argv[3]);
    strcpy(bufs,argv[4]);
    printf("Convert flag %s\nInput file %s\nOutput file %s\n\n",flg,inf,outf);
    flg[1] = tolower(flg[1]);
    if(0 == strcmp(flg,"-h")){
      printf("Making Intel Hex file.\n");
      i = b2h(inf,outf,bufs);
    }
    else {
      if(0 == strcmp(flg,"-b")){
	  printf("Making Binary file.\n");
	  i = h2b(inf,outf,bufs);
      }
      else {
      	printf("Flag Error!\n");
        i = 3;
      }
    }
    printf("Finished!\n");
    return i;
  }
  else { /*Error Infomation*/
    printf("Parameter Error!\n\nUsage: bhc flag input_file output_file ROM_size\n\n");
    printf("  flag: -h Convert to Intel Hex data.\n"
           "        -b Convert to Binary data.\n\n"
           "  ROM_size: 16k,32k,64k,128k,256k,512k,1024k\n\nTry again!\n");
    return -1;
  }
}

int b2h(char *in,char *out,char *bufs)/*Binary to Hex convert*/
{
  FILE *inp, *outp;
  int ch,cnt,csum,ofsa;
  struct stat statbuf;
  long int fsize,fpoint,fsub,adrs;

  /*Open file check*/
  if((inp = fopen(in, "rb")) == NULL){
    printf("Cannot open input file.\n");
    return 1;
  }
  if((outp = fopen(out, "wt")) == NULL){
    printf("Cannot open output file.\n");
    return 1;
  }
  fstat(fileno(inp), &statbuf);
  printf("Input file size=%lld\n",statbuf.st_size);
  fsize = (long int)statbuf.st_size;

  if(fsize > scnv(bufs)){ /* File size check*/
  	printf("File size error.\n");
	fclose(inp);
	fclose(outp);
    return 2;
  }

  cnt = 0; ofsa = 0; adrs = 0; fpoint = 0;
  fprintf(outp,":020000020000FC\n");/*Start Header*/
  fsub = fsize - fpoint;
  if (fsub > 0x20) {
	  fprintf(outp,":20%04X00",(unsigned int)adrs);/*Hex line Header*/
    csum = 0x20 + (adrs>>8) + (adrs & 0xFF);
    adrs = 0x20;
  }
  else {
	  fprintf(outp, ":%02X%04X00", (unsigned int)fsub,(unsigned int)adrs);/*Hex line Header*/
    csum = fsub + (adrs>>8) + (adrs & 0xFF);
    adrs = fsub;
  }
  while (fsub > 0){
    ch = fgetc(inp);
    fprintf(outp,"%02X",ch);/*Put data*/
    cnt++; fpoint++;
    fsub = fsize - fpoint;
    csum = ch + csum;
    if((fsub == 0)||(cnt == 0x20)){
      cnt = 0; csum = 0xFF & (~csum + 1);
      fprintf(outp,"%02X\n",csum);/*Put checksum*/
      if(fsub == 0) break;
      if(adrs > 0xFFFF){
		ofsa = 0x1000 + ofsa;
		adrs = 0;
		fprintf(outp,":02000002%04X",(unsigned int)ofsa);/*Change offset address*/
		csum = 0x02 + 0x02 + (ofsa>>8) + (ofsa & 0xFF);
		csum = 0xFF & (~csum + 1);
        fprintf(outp,"%02X\n", csum);
      }
      adrs = 0xFFFF & adrs;
	  if (fsub > 0x20) {
		  fprintf(outp,":20%04X00",(unsigned int)adrs);/*Next Hex line Header*/
    	csum = 0x20 + (adrs>>8) + (adrs & 0xFF);
        adrs = adrs + 0x20;
      }
      else {
      	if(fsub > 0){
		fprintf(outp, ":%02X%04X00", (unsigned int)fsub,(unsigned int)adrs);/*Next Hex line Header*/
    		csum = fsub + (adrs>>8) + (adrs & 0xFF);
        	adrs = adrs + fsub;
        }
      }
    }
  }
  fprintf(outp,":00000001FF\n");/*End footer*/

  fstat(fileno(outp), &statbuf);
  printf("Output file size=%lld\n",statbuf.st_size);

  fclose(inp);
  fclose(outp);
  return 0;
}

int h2b(char *in,char *out,char *bufs)/*Hex to Binary convert*/
{
  FILE *inp, *outp;
  int lsize,dmode;
  char buf[1048576],lbuf[256];
  struct stat statbuf;
  long fsize,po,adrs,ofsa,abadrs;

  /*Open file check*/
  if((inp = fopen(in, "rt")) == NULL){
    printf("Cannot open input file.\n");
    return 1;
  }
  if((outp = fopen(out, "wb")) == NULL){
    printf("Cannot open output file.\n");
    return 1;
  }
  fstat(fileno(inp), &statbuf);
  printf("Input file size=%lld\n",statbuf.st_size);

  fsize = scnv(bufs);

  for(po = 0;po < 1048576;po++) buf[po] = 0xFF;/*Fill Buffer 0xFF data*/

  adrs = 0; ofsa = 0;
  while (NULL != fgets(lbuf,254,inp)){/*Loop until end of file*/
        if((lbuf[0] != ':')||(!isalnum(lbuf[1]))||(!isalnum(lbuf[2]))) {
    	continue;
    }
    lsize = ch2num(lbuf[1],lbuf[2]) * 2;
    adrs = (long)(ch2num(lbuf[3],lbuf[4])) * 0x100L 
		+ (long)(ch2num(lbuf[5],lbuf[6]));
    if(fsize < (ofsa + adrs)) {/*Address check*/
    	printf("Address error!\n");
		fclose(inp);
		fclose(outp);
        return 5;
    }

    dmode = ch2num(lbuf[7],lbuf[8]);/*Get data mode */
    if(dmode == 2) {/*Get offset address*/
    	ofsa = ((long)ch2num(lbuf[9],lbuf[10]) * 0x100L
		+ (long)ch2num(lbuf[11],lbuf[12])) * 0x10L;
        continue;
    }
    if(dmode == 0){/*Get binary data*/
        abadrs = ofsa + adrs;
        for(po = 9;po < (lsize+9);po=po+2){
	        buf[abadrs++] = ch2num(lbuf[po],lbuf[po+1]);/*Write data to buffer*/
        }
    }
  }

  for(po = 0;po < fsize;po++){/*Write buffer data to file*/
  	putc(buf[po],outp);
  }

  fstat(fileno(outp), &statbuf);
  printf("Output file size=%lld\n",statbuf.st_size);

  fclose(inp);
  fclose(outp);
  return 0;
}

int scnv(char *size_ch)/* Size character converte to Integer*/
{
  if(strcmp(size_ch,"16k") == 0) return 16384;
  if(strcmp(size_ch,"32k") == 0) return 32768;
  if(strcmp(size_ch,"64k") == 0) return 65536;
  if(strcmp(size_ch,"128k") == 0) return 131072;
  if(strcmp(size_ch,"256k") == 0) return 262144;
  if(strcmp(size_ch,"512k") == 0) return 524288;
  if(strcmp(size_ch,"1024k") == 0) return 1048576;
  return  1048576;
}

int ch2num(char ch1,char ch2)
{
  int nout;

  if((ch1 >= '0')&&('9' >= ch1)) nout = (ch1 & 0xf) * 0x10;
  else nout = ((ch1 & 0xf) + 9) * 0x10;
  if((ch2 >= '0')&&('9' >= ch2)) return (ch2 & 0xf) + nout;
  else return ((ch2 & 0xf) + 9) + nout;
}
/* Thanks to KITA (QZE01066@nifty.ne.jp) */
/* End of file*/
