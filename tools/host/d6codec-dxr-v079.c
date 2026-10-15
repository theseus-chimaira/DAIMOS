/* Staged TXG executable producer; never substitutes production d6lz -x. */
#include "d6codec-group-v071.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MASK UINT64_C(0777777777777)
#define HALF UINT64_C(0777777)
#define COMP UINT64_C(0100000)
#define TX2 UINT64_C(0647022)
#define TXG UINT64_C(0647047)
#define DXR UINT64_C(0447062)
static int load(const char *path, uint64_t **data, size_t *count)
{
    FILE *f=fopen(path,"rb"); long length; size_t i; uint64_t *p;
    if (!f || fseek(f,0,SEEK_END) || (length=ftell(f))<0 || length%8 ||
        fseek(f,0,SEEK_SET)) return -1;
    *count=(size_t)length/8;
    p=malloc((*count ? *count : 1)*sizeof(*p));
    if (!p) return -1;
    for (i=0;i<*count;i++) {
        unsigned char b[8]; unsigned j;
        if (fread(b,1,8,f)!=8) { free(p); return -1; }
        p[i]=0;
        for (j=0;j<8;j++) p[i]|=(uint64_t)b[j]<<(8*j);
        if (p[i]>MASK) { free(p); return -1; }
    }
    if (fclose(f)) { free(p); return -1; }
    *data=p;return 0;
}
static int save(const char *path,const uint64_t *data,size_t n)
{
    FILE *f=fopen(path,"wb");size_t i;
    if (!f) return -1;
    for (i=0;i<n;i++) {
        unsigned char b[8];unsigned j;
        for (j=0;j<8;j++) b[j]=(unsigned char)(data[i]>>(8*j));
        if (fwrite(b,1,8,f)!=8) {fclose(f);return -1;}
    }
    return fclose(f)==0 ? 0:-1;
}
int main(int argc,char **argv)
{
    uint64_t *src=NULL,*dst=NULL;size_t n,image,reloc,header,cap,packed,words;
    int rc=1;
    if (argc!=3 || strcmp(argv[1],argv[2])==0) goto done;
    if (load(argv[1],&src,&n) || n<2 || (src[0]>>18)!=DXR ||
        (src[1]&COMP)) goto done;
    image=(size_t)(src[1]>>18);
    if (!image || image>0300000 || (src[0]&HALF)>=image) goto done;
    reloc=(image+35)/36;
    if (n==2+image+reloc) header=2;
    else if (n==3+image+reloc && (src[2]&HALF)==TX2 &&
             (src[2]>>18)<=image) header=3;
    else goto done;
    /* Bounded encoder reserves 15 descriptor words per group. */
    if (image>(SIZE_MAX-3-reloc)/2) goto done;
    cap=3+2*image+reloc;
    dst=calloc(cap,sizeof(*dst));
    if (!dst) goto done;
    if (d6codec_group_v072_encode(src+header,image,dst+3,
        cap-3-reloc,&packed)) goto done;
    dst[0]=src[0];dst[1]=src[1]|COMP;
    dst[2]=(header==3 ? (src[2]&(~HALF)) : 0)|TXG;
    memcpy(dst+3+packed,src+header+image,reloc*sizeof(*src));
    words=3+packed+reloc;
    if (save(argv[2],dst,words)) goto done;
    rc=0;
done:
    if (rc) fputs("d6codec-dxr-v079: invalid DXR, encoding, or I/O\n",stderr);
    free(src);free(dst);return rc;
}
