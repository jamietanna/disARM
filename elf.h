//
//  elf.h
//  DisARM
//
//

#ifndef DisARM_elf_h
#define DisARM_elf_h

typedef struct _elfHeader {
    unsigned char magic[4];
    unsigned char class;
    unsigned char byteorder;
    unsigned char hversion;
    unsigned char pad[9];

    unsigned short filetype;
    unsigned short archtype;

    unsigned int fversion;
    unsigned int entry;
    unsigned int phdrpos;
    unsigned int shdrpos;
    unsigned int flags;

    unsigned short hdrsize;
    unsigned short phdrent;
    unsigned short phdrcnt;
    unsigned short shdrent;
    unsigned short shdrcnt;
    unsigned short strsec;


} ELFHEADER;

typedef struct _elfProgHeader {
    unsigned int type;
    unsigned int offset;
    unsigned int virtaddr;
    unsigned int physaddr;
    unsigned int filesize;
    unsigned int memsize;
    unsigned int flags;
    unsigned int align;

} ELFPROGHDR;

typedef struct _elfSectHeader {
    unsigned int name;
    unsigned int type;
    unsigned int flags;
    unsigned int address;
    unsigned int offset;
    unsigned int size;
    unsigned int link;
    unsigned int info;
    unsigned int addralign;
    unsigned int entsize;
} ELFSECTHEADER;

typedef struct _elfSymbTable {
    unsigned int name;
    unsigned int value;
    unsigned int size;
    unsigned char info;
    unsigned char other;
    unsigned short shndx;
} ELFSYMTABLE;

#endif
