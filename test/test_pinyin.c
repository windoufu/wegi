/*----------------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

A test for converting Pinyin to Unicode han.

With reference to:
  1. https://blog.csdn.net/firehood_/article/details/7648625
  2. https://blog.csdn.net/FortuneWang/article/details/41575039
  3. http://www.unicode.org/Public/UCD/latest/ucd/Unihan.zip (Unihan_Readings.txt)
  4. http://www.unicode.org/reports/tr38


			--- Definition and glossary ---

Reading:   The pronunciations for a given character in written form.
	   Example:  chuǎng	yīn  (kMandarin)
Pinyin:    Modified reading for keyboard input.
	   Example:  chuang3   yin1  ( With a digital number to indicate the tone )


Note:
1. Prepare kHanyuPinyin.txt
	cat Unihan_Readings.txt | grep kHanyuPinyin > kHanyuPinyin.txt

   File 'kHanyuPinyin.txt' contains unicode, dict page number and mandarin reading(original pinyin):
	... ...
	U+92E7	kHanyuPinyin	64207.100:xiàn
	U+92E8	kHanyuPinyin	64208.110:tiě,é
	... ....


2. Extract unicodes and reading to put to an EGI_UNIHAN.


Midas Zhou
----------------------------------------------------------------------------------*/
#include "egi_common.h"
#include "egi_FTsymbol.h"
#include "egi_cstring.h"
#include "egi_unihan.h"
#include <errno.h>


/*===================
	MAIN
===================*/
int main(void)
{
	EGI_UNIHAN_SET	*uniset=NULL;
	EGI_UNIHAN_SET  *uniset3500=NULL;
	EGI_UNIHAN_SET  *tmpset=NULL;
	char pch[4];
	int i,k;
	struct timeval	tm_start,tm_end;
	wchar_t  	swcode;

	/* Load sysfont */
        printf("FTsymbol_load_sysfonts()...\n");
        if(FTsymbol_load_sysfonts() !=0) {
                printf("Fail to load FT sysfonts, quit.\n");
                return -1;
        }

	/* Init FBDEV */
        printf("init_fbdev()...\n");
        if( init_fbdev(&gv_fb_dev) )            /* init sys FB */
                return -1;

        /* Set sys FB mode */
        fb_set_directFB(&gv_fb_dev,true);
        fb_position_rotate(&gv_fb_dev,0);


#if 0	/* ------------- Strange wcode -------------- */
	int m, pos;
	wchar_t wcode;
	//char *stranges="ê̄,ế,éi,ê̌,ěi,ề,èi";   /* ê̄,ế,ê̌,ề   ế =[0x1ebf]  ê̄ =[0xea  0x304]  ê̌ =[0xea  0x30c] ề =[0x1ec1] */
	char *stranges="ńňǹḿ m̀"; /*   0x144  0x148  0x1f9   ḿ =[0x1e3f  0x20]  m̀=[0x6d  0x300] */
	printf("stranges: %s\n", stranges);
	m=0; pos=0;
	while( (m=char_uft8_to_unicode( (const UFT8_PCHAR)(stranges+pos), &wcode)) > 0 )
	{
		pos+=m;
		printf("0x%x  ", wcode);
	}
	printf("\n");
	exit(1);
#endif




#if 0 /*-------------  1. test INSERT_SORT tmpset  ------------------*/
	/* Read txt  into EGI_UNIHAN_SET */
	tmpset=UniHan_load_HanyuPinyinTxt(HANYUPINYIN_TXT_PATH);
	if( tmpset==NULL )
		exit(1);

        printf("tmpset total size=%d:\n", tmpset->size);
        for(i=0; i< tmpset->size; i++) {
		bzero(pch,sizeof(pch));
	    //if(uniset->unihans[i].wcode==0x54B9)
        	printf("[%s:%d]%s ",char_unicode_to_uft8(&(tmpset->unihans[i].wcode), pch)>0?pch:" ",
								tmpset->unihans[i].wcode, tmpset->unihans[i].typing);
	}
        printf("\n");

	/* Insert sort tmpset */
	printf("Start insertSort() UNISET typings...\n");
	gettimeofday(&tm_start,NULL);
	UniHan_insertSort_typing(tmpset->unihans, tmpset->size);
	gettimeofday(&tm_end,NULL);
        printf("OK! Finish insert_sorting, total size=%d, cost time=%ldms.\n", uniset->size, tm_diffus(tm_start, tm_end)/1000);

	/* Check result tmpset */
	printf("Check results of insertSort() tmpset.\n");
	for(i=0; i< uniset->size; i++) {
		#if 0
		bzero(pch,sizeof(pch));
       		printf("[%s:%d]%s",char_unicode_to_uft8(&(tmpset->unihans[i].wcode), pch)>0?pch:" ",
								tmpset->unihans[i].wcode, tmpset->unihans[i].typing);
		#endif
		if( i>0 && UniHan_compare_typing(tmpset->unihans+i, tmpset->unihans+i-1) == CMPTYPING_IS_AHEAD )
				printf("\n------------- i=%d, tmpset[] ERROR ---------------\n",i);

	}
        printf("OK. \n");

	/* Display some sorted unihans */
	#if 1
	printf("============ tmpset ===============\n");
	for( i=12000; i<12100; i++ ) {
			bzero(pch,sizeof(pch));
        		printf("unihans[%d]: [%s:%d]%s\n", i, char_unicode_to_uft8(&(tmpset->unihans[i].wcode), pch)>0?pch:" ",
								tmpset->unihans[i].wcode, tmpset->unihans[i].typing);
	}
	#endif

#endif  /*-------- END Insert_sort tmpset -------- */






#if 1 /*-------------- 2. test QUICK_SORT uniset ------------------*/
	/* Read kHanyuPinyin txt into uniset */
	uniset=UniHan_load_HanyuPinyinTxt(HANYUPINYIN_TXT_PATH);
	if( uniset==NULL )
		exit(1);
        printf("Load kHanyuPinyin txt to uniset, total size=%d:\n", uniset->size);
	getchar();

	/* Load saved uniset3500 */
        uniset3500=UniHan_load_uniset(PINYIN3500_DATA_PATH);
        if(uniset==NULL)
                exit(2);
        else
                printf("Load %s to uniset, name='%s', size=%d\n",PINYIN3500_DATA_PATH, uniset->name, uniset->size);

	/* merge uniset3500 into uniset */
	UniHan_merge_uniset(uniset3500, uniset);
	getchar();

	/* check merged uniset */
	UniHan_check_uniset(uniset);
	getchar();

	/* TEST: Locate a wcode */
	#if 0
	swcode=0x4E00; //0x9FA4;
	if( UniHan_locate_wcode(uniset,swcode) !=0 ) //9FA5); //x54B9);
		printf("swcode NOT found!\n");
	while( uniset->unihans[uniset->puh].wcode==swcode ) {
		bzero(pch,sizeof(pch));
        	printf("[%s:%d]%s\n",char_unicode_to_uft8(&(uniset->unihans[uniset->puh].wcode), pch)>0?pch:" ",
								uniset->unihans[uniset->puh].wcode, uniset->unihans[uniset->puh].typing);

		uniset->puh++;
	}
	exit(1);
	#endif

        /*  Quick_Sort with KEY==wcode before poll freq  */
        printf("Start quick_sorting by wcode..."); fflush(stdout);
        UniHan_quickSort_wcode( uniset->unihans, 0, uniset->size-1, 10);
        printf("...OK\n");

        /* Poll frequence by text, BEFORE quickSort_typing() */
        UniHan_reset_freq( uniset);

	#if 0
        printf("开始统计<红楼梦>字符频率 ..."); fflush(stdout);
        UniHan_poll_freq(uniset, "/mmc/hlm_all.txt");
        printf("开始统计<西游记>字符频率 ..."); fflush(stdout);
        UniHan_poll_freq(uniset, "/mmc/xyj_all.txt");
        printf("开始统计<三国演义>字符频率 ..."); fflush(stdout);
        UniHan_poll_freq(uniset, "/mmc/sgyy_all.txt");
	#endif
	#if 1
        printf("开始统计<水浒传>字符频率 ..."); fflush(stdout);
        UniHan_poll_freq(uniset, "/mmc/shz_all.txt");
	#endif
        printf("...OK\n");

        /* Quick Sort uniset with KEY==frequency */
        printf("Start quick_sorting by freq..."); fflush(stdout);
        UniHan_quickSort_freq( uniset->unihans, 0, uniset->size-1, 10);
        printf("...OK\n");

	#if 1 /* Check repeated case */
	swcode=0x513f;
        for(i=0; i< uniset->size; i++) {
		bzero(pch,sizeof(pch));
	    if(uniset->unihans[i].wcode==swcode) //0x4E00) //0x9FA4) //0x9FA5)
        	printf("i=%d, [%s:0x%04x]%s ",i,char_unicode_to_uft8(&(uniset->unihans[i].wcode), pch)>0?pch:" ",
								uniset->unihans[i].wcode, uniset->unihans[i].typing);
	}
        printf("\n");
	getchar();
	#endif

        /* Print 20 most frequently used UNIHANs in the text */
        i=0; k=0;
        while( k<30 )
        {
                /* Skip polyphonic unihans ()() still exeption: 儿[0x513f](6079) 玉[0x7389](6079) 儿[0x513f](6079)  */
                if(  uniset->size < 30
                     || uniset->unihans[uniset->size-i-1].wcode != uniset->unihans[uniset->size-i].wcode )
                {
                        bzero(pch,sizeof(pch));
                        printf("%d: %s[0x%04x](%d)\n", k+1, char_unicode_to_uft8(&(uniset->unihans[uniset->size-i-1].wcode), pch)>0 ? pch : "#",
                                                uniset->unihans[uniset->size-i-1].wcode, uniset->unihans[uniset->size-i-1].freq );
                        k++;
                }
                i++;
        }
        getchar();

	#if 1
	swcode=9712;
	for(i=0; i<uniset->size-1; i++) {
		if( swcode==uniset->unihans[i].wcode ) {
		                bzero(pch,sizeof(pch));
	                printf("[%s:%d(%d)]%s\n",char_unicode_to_uft8(&(uniset->unihans[i].wcode), pch)>0?pch:" ",
                                                                uniset->unihans[i].wcode, uniset->unihans[i].freq, uniset->unihans[i].typing);
		}
	}

        UniHan_quickSort_wcode( uniset->unihans, 0, uniset->size-1, 10);
	if( UniHan_locate_wcode(uniset,swcode) !=0 )
		printf("swcode NOT found!\n");
	while( uniset->unihans[uniset->puh].wcode==swcode ) {
		bzero(pch,sizeof(pch));
        	printf("[%s:%d]%s\n",char_unicode_to_uft8(&(uniset->unihans[uniset->puh].wcode), pch)>0?pch:" ",
								uniset->unihans[uniset->puh].wcode, uniset->unihans[uniset->puh].typing);

		uniset->puh++;
	}
	exit(1);
	#endif



	/* Quick sort uniset with KEY=typing */
	printf("Start quickSort() UNISET typings..."); fflush(stdout);
	gettimeofday(&tm_start,NULL);
	UniHan_quickSort_typing(uniset->unihans, 0, uniset->size-1, 10);
	gettimeofday(&tm_end,NULL);
        printf("OK! Finish quick_sorting, total size=%d, cost time=%ldms.\n", uniset->size, tm_diffus(tm_start, tm_end)/1000);

	/* Check result uniset */
	printf("Check results of quickSort() uniset..."); fflush(stdout);
	for(i=0; i< uniset->size; i++) {
		#if 1
		bzero(pch,sizeof(pch));
       		printf("[%s:%d]%s",char_unicode_to_uft8(&(uniset->unihans[i].wcode), pch)>0?pch:" ",
								uniset->unihans[i].wcode, uniset->unihans[i].typing);
		#endif
		if( i>0 && UniHan_compare_typing(uniset->unihans+i, uniset->unihans+i-1) == CMPTYPING_IS_AHEAD )
				printf("\n------------- i=%d, uniset[] ERROR ---------------\n",i);

	}
        printf("\nOK. \n");

	/* Display some sorted unihans */
	#if 0
	printf("============ uniset ===============\n");
	for( i=12000; i<12100; i++ ) {
			bzero(pch,sizeof(pch));
        		printf("unihans[%d]: [%s:%d]%s\n", i, char_unicode_to_uft8(&(uniset->unihans[i].wcode), pch)>0?pch:"#", /* # empty unihan */
								uniset->unihans[i].wcode, uniset->unihans[i].typing);
	}
	#endif

#endif  /*-------- END quick_sort uniset -------- */




#if 0  /* ---------  3. Compare tmpset and uniset as results of insertSort() AND quickSort() ------- */
	printf("Compare results of insertSort() and quickSort()\n");
	for(i=0; i< uniset->size; i++) {
		if( strcmp(tmpset->unihans[i].reading, uniset->unihans[i].reading)!=0 )
		{
			printf("Diff unihans[%d]: uniset ", i);
			bzero(pch,sizeof(pch));
        		printf("[%s:%d]%s ",char_unicode_to_uft8(&(uniset->unihans[i].wcode), pch)>0?pch:" ",
								uniset->unihans[i].wcode, uniset->unihans[i].typing);
			printf(": tmpset ");
			bzero(pch,sizeof(pch));
        		printf("[%s:%d]%s ",char_unicode_to_uft8(&(tmpset->unihans[i].wcode), pch)>0?pch:" ",
								tmpset->unihans[i].wcode, tmpset->unihans[i].typing);
			printf("\n");

		}
	}
	printf("OK\n");
#endif

	return 0;
}
