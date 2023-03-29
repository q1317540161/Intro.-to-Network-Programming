 #include<stdio.h> 
 #include<fcntl.h>
 #include<unistd.h>
 #include<stdint.h>
 #include<string.h>
 #include<stdlib.h>
 
//  my_CUR = lseek(input_fd, 0, SEEK_CUR);
//	printf("%d\n", my_CUR);

//	int output_fd = open("output.txt", O_WRONLY);
//	write(output_fd, h_stroff, sizeof(h_stroff));

 uint32_t ltob32(uint32_t src) {
    uint32_t temp = 0;
    temp += (src & 0x000000ff)<<3*8;
    temp += (src & 0x0000ff00)<<1*8;
    temp += (src & 0x00ff0000)>>1*8;
    temp += (src & 0xff000000)>>3*8;
    return temp;
 }
 uint64_t ltob64(uint64_t src) {
    uint64_t temp = 0;
    temp += (src & 0x00000000000000ff)<<7*8;
    temp += (src & 0x000000000000ff00)<<5*8;
    temp += (src & 0x0000000000ff0000)<<3*8;
    temp += (src & 0x00000000ff000000)<<1*8;
    temp += (src & 0x000000ff00000000)>>1*8;
    temp += (src & 0x0000ff0000000000)>>3*8;
    temp += (src & 0x00ff000000000000)>>5*8;
    temp += (src & 0xff00000000000000)>>7*8;

    return temp;
 }

 typedef struct {
        uint32_t magic;
        int32_t  off_str;
        int32_t  off_dat;
        uint32_t n_files;
} __attribute((packed)) header;

typedef struct {
    int32_t off_fname;
    uint32_t fsize;
    int32_t off_fcontent;
    uint64_t checksum;
}__attribute((packed)) FILE_E;

 int main(int argc, char* argv[])
 {
    int input_fd = open(argv[1],O_RDONLY);
    header mheader;
    FILE_E mfentry;

    memset(&mheader, 0, sizeof(mheader)); 
    
	read(input_fd, &mheader, sizeof(mheader));
    printf("n_files:%d\n", mheader.n_files);
    
    off_t entry = lseek(input_fd, 0, SEEK_CUR);

    for(int i=0; i<mheader.n_files; i++){
        //get file entry
        memset(&mfentry, 0, sizeof(mfentry));
        lseek(input_fd, entry+i*sizeof(FILE_E), SEEK_SET);
        read(input_fd, &mfentry,sizeof(FILE_E));
        
        mfentry.fsize = ltob32(mfentry.fsize);
        mfentry.checksum = ltob64(mfentry.checksum);

        //getname
        char fname[20];
        lseek(input_fd, mheader.off_str+mfentry.off_fname, SEEK_SET);
        for(int j=0;;j++){
            read(input_fd, &fname[j], sizeof(char));
            if(fname[j]=='\0') break;
        }
        printf("%d file: %s\n", i, fname);

        //checksum
        lseek(input_fd, mheader.off_dat+mfentry.off_fcontent, SEEK_SET);

        uint64_t check = mfentry.checksum;
        uint64_t tmp = 0;

        printf("file size: %x\n", mfentry.fsize);
        int32_t fsize = mfentry.fsize;
        while(fsize>8){
            tmp = 0;
            read(input_fd, &tmp, sizeof(tmp));
            fsize -=8;
            check = check^tmp;
        }
        if(fsize>0){
            tmp = 0;
            read(input_fd, &tmp, fsize);
            check = check^tmp;
            // printf("tmp = %016x\n", tmp);
        }

        // printf("check calc: %llx\n", check);
        printf("off_fcontent: %llx\n", mfentry.off_fcontent);
        printf("check real: %llx\n", mfentry.checksum);

        //content
        lseek(input_fd, mheader.off_dat+mfentry.off_fcontent, SEEK_SET);
        char* content;
        content = (char *)malloc(mfentry.fsize);
        read(input_fd, content, mfentry.fsize);

        char path[50];
        memset(path, '\0', sizeof(path));
        // strcat(path, ".\\inplab2test\\");
        strcat(path, argv[2]);
        strcat(path, "//");
        strcat(path, fname);

        if(check==0){
            FILE* output = fopen(path,"w");
            for (int i = 0; i < mfentry.fsize; i++) {
                fprintf(output, "%c", content[i]);
            }
            fclose(output);
        }
        if(check==0) printf("true\n\n");
        else printf("file false\n\n");
        
        free(content);
    }

    return 0;
 }
