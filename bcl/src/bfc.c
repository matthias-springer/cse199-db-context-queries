/*************************************************************************
* Name:        bfc.c
* Author:      Marcus Geelnard
* Description: Basic Compression Library file compressor.
*
* BFC stands for Basic File Compressor.
*
* This is a simple test application for all the compression types
* supported by the Basic Compression Library. It compresses or
* decompresses an input file with a selected compression algorithm. In no
* way is this program intended to be used for general purpose compression.
* It is just a testbed for the the different algorithms supported by the
* Basic Compression Library.
*
*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rle.h"
#include "shannonfano.h"
#include "huffman.h"
#include "rice.h"
#include "lz.h"
#include <algorithm>


/*************************************************************************
* ReadWord32()
*************************************************************************/

int ReadWord32( FILE *f )
{
  unsigned char buf[4];
  fread( buf, 4, 1, f );
  return (((unsigned int)buf[0])<<24) +
         (((unsigned int)buf[1])<<16) +
         (((unsigned int)buf[2])<<8)  +
           (unsigned int)buf[3];
}


/*************************************************************************
* WriteWord32()
*************************************************************************/

void WriteWord32( int x, FILE *f )
{
  fputc( (x>>24)&255, f );
  fputc( (x>>16)&255, f );
  fputc( (x>>8)&255, f );
  fputc( x&255, f );
}


/*************************************************************************
* GetFileSize()
*************************************************************************/

long GetFileSize( FILE *f )
{
    long pos, size;

    pos = ftell( f );
    fseek( f, 0, SEEK_END );
    size = ftell( f );
    fseek( f, pos, SEEK_SET );

    return size;
}


/*************************************************************************
* Help()
*************************************************************************/

void Help( char *prgname )
{
    printf( "Usage: %s command [algo] infile outfile\n\n", prgname );
    printf( "Commands:\n" );
    printf( "  c       Compress\n" );
    printf( "  d       Deompress\n\n" );
    printf( "Algo (only specify for compression):\n" );
    printf( "  rle     RLE Compression\n" );
    printf( "  sf      Shannon-Fano compression\n" );
    printf( "  huff    Huffman compression\n" );
    printf( "  lz      LZ77 Compression\n" );
    printf( "  rice8   Rice compresison of 8-bit data\n" );
    printf( "  rice16  Rice compresison of 16-bit data\n" );
    printf( "  rice32  Rice compresison of 32-bit data\n" );
    printf( "  rice8s  Rice compresison of 8-bit signed data\n" );
    printf( "  rice16s Rice compresison of 16-bit signed data\n" );
    printf( "  rice32s Rice compresison of 32-bit signed data\n" );
}

void gen_data(void** memptr, int lists, int size_each)
{
#define DTYPE unsigned short

  *memptr = malloc(lists*size_each*sizeof(DTYPE));
  DTYPE *arr = (DTYPE*) *memptr;

  for (int i = 0; i < lists*size_each; ++i)
  {
    arr[i] = rand() % 25000;
  }

  for (int i = 0; i < lists*size_each; i += size_each)
  {
    std::sort(arr + i, arr + i + size_each);

    int last = 0;
    for (int j = 0; j < size_each; ++j)
    {
      int next_last = arr[i + j];
      arr[i + j] = next_last - last;
      last = next_last;
    }
  }
}

/*************************************************************************
* main()
*************************************************************************/

int main( int argc, char **argv )
{
    FILE *f;
    unsigned char *in, *out, command, algo=0;
    unsigned int  insize, outsize=0, *work;
    char *inname, *outname;

    /* Check arguments */
    if( argc < 4 )
    {
        Help( argv[ 0 ] );
        return 0;
    }

    /* Get command */
    command = argv[1][0];
    if( (command != 'c') && (command != 'd') )
    {
        Help( argv[ 0 ] );
        return 0;
    }

    /* Get algo */
    if( argc == 5 && command == 'c' )
    {
        algo = 0;
        if( strcmp( argv[2], "rle" ) == 0 )     algo = 1;
        if( strcmp( argv[2], "huff" ) == 0 )    algo = 2;
        if( strcmp( argv[2], "rice8" ) == 0 )   algo = 3;
        if( strcmp( argv[2], "rice16" ) == 0 )  algo = 4;
        if( strcmp( argv[2], "rice32" ) == 0 )  algo = 5;
        if( strcmp( argv[2], "rice8s" ) == 0 )  algo = 6;
        if( strcmp( argv[2], "rice16s" ) == 0 ) algo = 7;
        if( strcmp( argv[2], "rice32s" ) == 0 ) algo = 8;
        if( strcmp( argv[2], "lz" ) == 0 )      algo = 9;
        if( strcmp( argv[2], "sf" ) == 0 )      algo = 10;
        if( !algo )
        {
            Help( argv[ 0 ] );
            return 0;
        }
        inname  = argv[ 3 ];
        outname = argv[ 4 ];
    }
    else if( argc == 4 && command == 'd' )
    {
        inname  = argv[ 2 ];
        outname = argv[ 3 ];
    }
    else
    {
        Help( argv[ 0 ] );
        return 0;
    }

    int lists = 2000;
    int size_each = 100;
    void* memory;

    gen_data(&memory, lists, size_each);

    /* Get input file size */
    insize = sizeof(DTYPE) * lists * size_each;

    /* Decompress? */
    if( command == 'd' )
    {
        /* Read header */
        algo = ReadWord32( f );  /* Dummy */
        algo = ReadWord32( f );
        outsize = ReadWord32( f );
        insize -= 12;
    }

    /* Print operation... */
    switch( algo )
    {
        case 1: printf( "RLE " ); break;
        case 2: printf( "Huffman " ); break;
        case 3: printf( "Rice 8-bit " ); break;
        case 4: printf( "Rice 16-bit " ); break;
        case 5: printf( "Rice 32-bit " ); break;
        case 6: printf( "Rice 8-bit signed " ); break;
        case 7: printf( "Rice 16-bit signed " ); break;
        case 8: printf( "Rice 32-bit signed " ); break;
        case 9: printf( "LZ77 " ); break;
        case 10: printf( "Shannon-Fano " ); break;
    }
    switch( command )
    {
        case 'c': printf( "compress " ); break;
        case 'd': printf( "decompress " ); break;
    }
    printf( "%s to %s...\n", inname, outname );


    in = (unsigned char *) memory;
  printf("Insize: %i\n", insize);


    /* Show output file size for decompression */
    if( command == 'd' )
    {
        printf( "Output file: %d bytes\n", outsize );
    }


    /* Open output file */
    //f = fopen( outname, "wb" );
    //if( !f )
    //{
     //   printf( "Unable to open output file \"%s\".\n", outname );
    //    free( in );
    //    return 0;
   // }

    /* Compress? */
    if( command == 'c' )
    {
     //   /* Write header */
     //   fwrite( "BCL1", 4, 1, f );
     //   WriteWord32( algo, f );
     //   WriteWord32( insize, f );

        /* Worst case buffer size */
     //   outsize = (insize*104+50)/100 + 384;
    }

    /* Allocate memory for output buffer */
    out = (unsigned char*) malloc( outsize );
    if( !out )
    {
        printf( "Not enough memory\n" );
        fclose( f );
        free( in );
        return 0;
    }

    /* Compress or decompress */
    if( command == 'c' )
    {
        switch( algo )
        {
            case 1:
                outsize = RLE_Compress( in, out, insize );
                break;
            case 2:
                outsize = Huffman_Compress( (unsigned short*) in, (unsigned short *) out, insize );
                break;
            case 3:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_UINT8 );
                break;
            case 4:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_UINT16 );
                break;
            case 5:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_UINT32 );
                break;
            case 6:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_INT8 );
                break;
            case 7:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_INT16 );
                break;
            case 8:
                outsize = Rice_Compress( in, out, insize, RICE_FMT_INT32 );
                break;
            case 9:
                work = (unsigned int*) malloc( sizeof(unsigned int) * (65536+insize) );
                if( work )
                {
                    outsize = LZ_CompressFast( in, out, insize, work );
                    free( work );
                }
                else
                {
                    outsize = LZ_Compress( in, out, insize );
                }
                break;
            case 10:
                outsize = SF_Compress( in, out, insize );
                break;
        }
        printf( "Output file: %d bytes (%.1f%%)\n", outsize,
                100*(float)outsize/(float)insize );
    }
    else
    {
        switch( algo )
        {
            case 1:
                RLE_Uncompress( in, out, insize );
                break;
            case 2:
                Huffman_Uncompress( (unsigned short*) in, (unsigned short*) out, insize, outsize );
                break;
            case 3:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_UINT8 );
                break;
            case 4:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_UINT16 );
                break;
            case 5:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_UINT32 );
                break;
            case 6:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_INT8 );
                break;
            case 7:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_INT16 );
                break;
            case 8:
                Rice_Uncompress( in, out, insize, outsize, RICE_FMT_INT32 );
                break;
            case 9:
                LZ_Uncompress( in, out, insize );
                break;
            case 10:
                SF_Uncompress( in, out, insize, outsize );
                break;
            default:
                printf( "Unknown compression algorithm: %d\n", algo );
                free( in );
                free( out );
                fclose( f );
                return 0;
        }
    }

    /* Write output file */
    //fwrite( out, outsize, 1, f );
    //fclose( f );

    /* Free memory */
    free( in );
    free( out );

    return 0;
}

