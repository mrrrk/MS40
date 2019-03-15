 /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/        
 /*@@       @   @@@@   @@@     @@@      @@@@       @       @@       @@*/        
 /*@   @@@@@@    @@    @@  @@  @@   @@   @@   @@@@@@  @@@   @  @@@   @*/        
 /*@  @@@   @  @    @  @        @  @@@@  @@  @@@   @       @@  @@@@  @*/        
 /*@   @@@  @  @@  @@  @@@@@@  @@   @@   @@   @@@  @  @@@@@@@  @@@   @*/        
 /*@@       @  @@@@@@  @@@@@@  @@@      @@@@       @  @@@@@@@       @@*/        
 /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/        
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   PROGRAM         :  GM40GPD                                  | */        
 /* |   DATE WRITTEN    :  September 1998     (cc 6855)             | */        
 /* |   PROGRAMMER      :  Mark Hodgson                             | */        
 /* |   FUNCTION        :  Generic update shell program             | */        
 /* |_______________________________________________________________| */        
 /* |                                                               | */        
 /* |   MODIFICATION    :                                           | */        
 /* |   DATE            :                                           | */        
 /* |   ANALYST         :                                           | */        
 /* |   PROGRAMMER      :                                           | */        
 /* |   REASON          :                                           | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 #include <stdlib.h>                                                            
 #include <stdio.h>                                                             
 #include <decimal.h>                                                           
 #include <string.h>                                                            
 #include <signal.h>                                                            
 #include <time.h>                                                              
 #include <ctest.h>                                                             
 #include <ctype.h>                                                             
 #include <limits.h>                                                            
 #include "fmslib.h"                                                            
 #include "genreprt.h"                                                          
 #include "GM40GPD.h"                                                           
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  structure tags (see header files also)                       | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 struct FilePointers                                                            
 {                                                                              
   FILE* pUpdFile;                                                              
   FILE* pMasFile;                                                              
   FILE* pCompFile;                                                             
   FILE* pParmFile;                                                             
   FILE* pMG64File;                                                             
   FILE* pDateFile;                                                             
   FILE* pMasOutFile;                                                           
   FILE* pUpdRejFile;                                                           
   FILE* pPrintFile;                                                            
 };                                                                             
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function prototypes                                          | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void errorHandler(int SIG_TYPE);                                        
                                                                                
 static void processProgParms(int parmCount,                                    
                              char** ppParms,                                   
                              struct ProgramParameters* pProgParms);            
                                                                                
 static void performInitialisation(struct ProgramParameters* pProgParms,        
                                   FMS_PARM_STRUCT** ppFmsParms,                
                                   struct FilePointers* pFilePtrs,              
                                   struct ReportStatistics* pRepStats,          
                                   char** ppOverflowArea);                      
                                                                                
 static void readCompanyFile(FILE* pCompFile,                                   
                             struct ProgramParameters* pProgParms,              
                             char* companyName);                                
                                                                                
 static void readMG64cards(FILE* pInFile,                                       
                           int numSegs,                                         
                           char* companyCode,                                   
                           struct ReportStatistics* pRepStats);                 
                                                                                
 static void initialiseRecordObject(struct RecordDetails* pFmsRec,              
                                   FMS_PARM_STRUCT* pFmsParms,                  
                                   char* dataArea,                              
                                   char format);                                
                                                                                
 static void readOasisRecord(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             FILE* pInFile,                                     
                             struct ReportStatistics* pRepStats,                
                             struct ProgramParameters* pProgParms,              
                             char* pOverflowArea);                              
                                                                                
 static size_t readOverflow(struct RecordDetails* pFmsRec,                      
                            FMS_PARM_STRUCT* pFmsParms,                         
                            char* pOverflowArea,                                
                            FILE* pInFile);                                     
                                                                                
 static int checkRecLength(struct RecordDetails* pFmsRec,                       
                           FMS_PARM_STRUCT* pFmsParms);                         
                                                                                
 extern void GM40GPDU(FMS_PARM_STRUCT* pFmsParms,                               
                      int segNum,                                               
                      char* pUpdSeg,                                            
                      char* pMasSeg,                                            
                      char* binSysDate);                                        
                                                                                
 static int compareUrns(struct RecordDetails* pFmsRec1,                         
                        struct RecordDetails* pFmsRec2);                        
                                                                                
 static void applyUpdateRecord(struct RecordDetails* pUpdRec,                   
                               struct RecordDetails* pMasRec,                   
                               FMS_PARM_STRUCT* pFmsParms,                      
                               struct ProgramParameters* pProgParms,            
                               struct ReportStatistics* pRepStats);             
                                                                                
 extern int GM40GPDK(char* pUpdSeg,                                             
                     int updSegNum,                                             
                     struct RecordDetails* pMasRec,                             
                     FMS_PARM_STRUCT* pFmsParms);                               
                                                                                
 static void fixClashingUpdatePointers(struct RecordDetails* pUpdRec,           
                                       struct RecordDetails* pMasRec,           
                                       FMS_PARM_STRUCT* pFmsParms);             
                                                                                
 static void rewireLinkBoxPointers(int segment,                                 
                                   char* pMasSeg,                               
                                   char* pUpdSeg,                               
                                   struct RecordDetails* pUpdRec,               
                                   struct RecordDetails* pMasRec,               
                                   FMS_PARM_STRUCT* pFmsParms);                 
                                                                                
 static void rewireUpdateSegPointers(int parentSeg,                             
                                     char* pMasSeg,                             
                                     char* pUpdSeg,                             
                                     struct RecordDetails* pUpdRec,             
                                     struct RecordDetails* pMasRec,             
                                     FMS_PARM_STRUCT* pFmsParms);               
                                                                                
 static void addSegment(int segNum,                                             
                        char* pUpdSeg,                                          
                        struct RecordDetails* pMasRec,                          
                        FMS_PARM_STRUCT* pFmsParms);                            
                                                                                
 static void checkForPossibleOverflow(struct RecordDetails* pUpdRec,            
                                      struct RecordDetails* pMasRec,            
                                      FMS_PARM_STRUCT* pFmsParms,               
                                      struct ProgramParameters* pProgParms,     
                                      struct ReportStatistics* pRepStats,       
                                      FILE* pUpdRejFile,                        
                                      char* pOverflowArea);                     
                                                                                
 static void writeOasisRecord(struct RecordDetails* pFmsRec,                    
                              FMS_PARM_STRUCT* pFmsParms,                       
                              FILE* pOutFile,                                   
                              struct ReportStatistics* pRepStats,               
                              struct ProgramParameters* pProgParms);            
                                                                                
 static void dedupeLinkBoxes(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             struct ReportStatistics* pRepStats);               
                                                                                
 static size_t writeOverflow(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             FILE* pOutFile);                                   
                                                                                
 static void performTermination(struct ReportStatistics* pRepStats,             
                                struct ProgramParameters* pProgParms,           
                                struct FilePointers* pFilePtrs,                 
                                FMS_PARM_STRUCT* pFmsParms);                    
                                                                                
 extern void GM40PDO(struct ProgramParameters** ppProgParms,                    
                      struct ReportStatistics** ppRepStats,                     
                      struct RecordDetails** ppFmsRec,                          
                      FMS_PARM_STRUCT** ppFmsParms,                             
                      int* runType);                                            
                                                                                
 static void printReports(struct ReportStatistics* pRepStats,                   
                          struct ProgramParameters* pProgParms,                 
                          struct FilePointers* pFilePtrs,                       
                          FMS_PARM_STRUCT* pFmsParms);                          
                                                                                
 /* for debugging */                                                            
 static void showData(char* segPtr, int segLen);                                
 static int showField(struct RecordDetails* pFmsRec,                            
                      int seg,                                                  
                      int occ,                                                  
                      int fld,                                                  
                      FMS_PARM_STRUCT* pFmsParms);                              
                                                                                
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : main                                              | */        
 /* |    - the main function (what else?)                           | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 int main (int parmCount, char **ppParms)                                       
 {                                                                              
                                                                                
   /* declare structure to store parameter details (defined in GM40GPD.h) */    
   static struct ProgramParameters progParms = {0};                             
                                                                                
   /* declare structure to store file pointers (defined above) */               
   static struct FilePointers filePtrs;                                         
                                                                                
   /* declare structure to store FMS parameters (defined in FMSLIB.h) */        
   static FMS_PARM_STRUCT* pFmsParms = NULL;                                    
                                                                                
   /* declare structure to store report counts etc. (defined in GM40GPD.h) */   
   static struct ReportStatistics repStats = {0};                               
                                                                                
   /* declare FMS record 'objects' (defined in GM40GPD.h) */                    
   static struct RecordDetails updRec = {0};                                    
   static struct RecordDetails masRec = {0};                                    
   static struct RecordDetails savedMasRec = {0};                               
                                                                                
   /* declare record storage areas.  note that first two bytes are */           
   /* reserved for varying strings in optional PL/1 subroutines.   */           
   /* the overflow area will be dynamically allocated later.       */           
   static char updArea1MAX_REC_SIZE+2# = {0};                                   
   static char masArea1MAX_REC_SIZE+2# = {0};                                   
   static char newMasArea1MAX_REC_SIZE+2# = {0};                                
   static char* overflowMasArea = NULL;                                         
                                                                                
   /* for detecting next URN */                                                 
   static char prevUpdUrn1100# = {0};                                           
                                                                                
 int ctl1=0, ctl2=0, ctl3=0;                                                    
 printf("Oh my God! They killed Kenny!\n");                                     
                                                                                
   signal(SIGABRT, errorHandler);                                               
                                                                                
   processProgParms(parmCount, ppParms, &progParms);                            
                                                                                
   performInitialisation(&progParms,                                            
                         &pFmsParms,                                            
                         &filePtrs,                                             
                         &repStats,                                             
                         &overflowMasArea);                                     
                                                                                
   /* joins record object with storage area initialises shortcut fields etc. */ 
   initialiseRecordObject(&updRec, pFmsParms, updArea+2, 'U');                  
   initialiseRecordObject(&masRec, pFmsParms, masArea+2, 'M');                  
                                                                                
   /* note that the optional routine GM40GPDI is called by this function */     
   readOasisRecord(&masRec,                                                     
                   pFmsParms,                                                   
                   filePtrs.pMasFile,                                           
                   &repStats,                                                   
                   &progParms,                                                  
                   overflowMasArea);                                            
                                                                                
   /* check for empty files and check rec lengths against paramater file... */  
                                                                                
   /* an empty master is fine if FIRSTRUN parameter is set */                   
   if (feof(filePtrs.pMasFile) && !progParms.isFirstRun)                        
   {                                                                            
     fprintf(stderr,"ERROR : master file is empty\n ");                         
     abort();                                                                   
   }                                                                            
                                                                                
   if (0 != checkRecLength(&masRec, pFmsParms) && !feof(filePtrs.pMasFile))     
   {                                                                            
     fprintf(stderr,                                                            
       "ERROR : length of 1st master record does not match parameter file\n "); 
     abort();                                                                   
   }                                                                            
                                                                                
   readOasisRecord(&updRec,                                                     
                   pFmsParms,                                                   
                   filePtrs.pUpdFile,                                           
                   &repStats,                                                   
                   &progParms,                                                  
                   NULL); /* oflow area ptr is null because no oflow for upd */ 
                                                                                
   if (feof(filePtrs.pUpdFile))                                                 
   {                                                                            
     fprintf(stderr,"ERROR : update file is empty\n ");                         
     abort();                                                                   
   }                                                                            
                                                                                
   if (0 != checkRecLength(&updRec, pFmsParms))                                 
   {                                                                            
     fprintf(stderr,                                                            
       "ERROR : length of 1st update record does not match parameter file\n "); 
     abort();                                                                   
   }                                                                            
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  main loop - match updates to masters                         | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
   while (!feof(filePtrs.pMasFile)                                              
       || !feof(filePtrs.pUpdFile))                                             
   {                                                                            
 //if (ctl1++ > 100) break;                                                     
                                                                                
     while(compareUrns(&masRec,&updRec) == 0)                                   
     {                                                                          
 //if (ctl2++ > 100) break;                                                     
       checkForPossibleOverflow(&updRec,                                        
                                &masRec,                                        
                                pFmsParms,                                      
                                &progParms,                                     
                                &repStats,                                      
                                filePtrs.pUpdRejFile,                           
                                overflowMasArea);                               
                                                                                
       applyUpdateRecord(&updRec, &masRec, pFmsParms, &progParms, &repStats);   
                                                                                
       readOasisRecord(&updRec,                                                 
                       pFmsParms,                                               
                       filePtrs.pUpdFile,                                       
                       &repStats,                                               
                       &progParms,                                              
                       NULL);                                                   
     }                                                                          
                                                                                
     if(compareUrns(&masRec,&updRec) > 0 && !feof(filePtrs.pUpdFile))           
     {                                                                          
 //printf("BRAND SPANKING NEW RECORD!\n--------------------------\n");          
       savedMasRec = masRec;                                                    
       initialiseRecordObject(&masRec, pFmsParms, newMasArea+2, 'M');           
                                                                                
       while(compareUrns(&savedMasRec, &updRec) > 0 && !feof(filePtrs.pUpdFile))
       {                                                                        
 //if (ctl3++ > 100) break;                                                     
                                                                                
         checkForPossibleOverflow(&updRec,                                      
                                  &masRec,                                      
                                  pFmsParms,                                    
                                  &progParms,                                   
                                  &repStats,                                    
                                  filePtrs.pUpdRejFile,                         
                                  overflowMasArea);                             
                                                                                
         applyUpdateRecord(&updRec, &masRec, pFmsParms, &progParms, &repStats); 
                                                                                
         memcpy(prevUpdUrn, updRec.pUrn, updRec.urnLen);                        
                                                                                
         readOasisRecord(&updRec,                                               
                         pFmsParms,                                             
                         filePtrs.pUpdFile,                                     
                         &repStats,                                             
                         &progParms,                                            
                         NULL);                                                 
                                                                                
         if (memcmp(prevUpdUrn, updRec.pUrn, updRec.urnLen) != 0)               
         {                                                                      
           writeOasisRecord(&masRec,                                            
                            pFmsParms,                                          
                            filePtrs.pMasOutFile,                               
                            &repStats,                                          
                            &progParms);                                        
                                                                                
           initialiseRecordObject(&masRec, pFmsParms, newMasArea+2, 'M');       
         }                                                                      
 //else                                                                         
 //printf("new mas check rec len result %d\n",checkRecLength(&masRec,pFmsParms))
       }                                                                        
       masRec=savedMasRec;                                                      
     }                                                                          
                                                                                
     if(compareUrns(&masRec,&updRec) < 0)                                       
     {                                                                          
       writeOasisRecord(&masRec,                                                
                        pFmsParms,                                              
                        filePtrs.pMasOutFile,                                   
                        &repStats,                                              
                        &progParms);                                            
                                                                                
       /* re-initialise in case last record used overflow area */               
       initialiseRecordObject(&masRec, pFmsParms, masArea+2, 'M');              
                                                                                
       readOasisRecord(&masRec,                                                 
                       pFmsParms,                                               
                       filePtrs.pMasFile,                                       
                       &repStats,                                               
                       &progParms,                                              
                       overflowMasArea);                                        
     }                                                                          
   } /* end of loop through files */                                            
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  finish up (yayyy!!!)...                                      | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
   performTermination(&repStats, &progParms, &filePtrs, pFmsParms);             
                                                                                
   return 0;                                                                    
                                                                                
 } /* END OF MAIN */                                                            
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : processProgParms                                  | */        
 /* |      - validate and store parameters passed to program        | */        
 /* |      - the first three parms are mandatory and fixed format   | */        
 /* |      - the remaining parameters are optional and may appear   | */        
 /* |        in any order                                           | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void processProgParms(int parmCount,                                    
                              char** ppParms,                                   
                              struct ProgramParameters* pProgParms)             
 {                                                                              
   int i;                                                                       
                                                                                
   /* validate and store mandatory parameters... */                             
                                                                                
   if (parmCount < 4)                                                           
   {                                                                            
     fprintf(stderr,"insufficient parameters\n");                               
     fprintf(stderr,"require: 1company# 1version# 1overflow max#\n");           
     abort();                                                                   
   }                                                                            
                                                                                
   if (strlen(ppParms11#) != 3)                                                 
   {                                                                            
     fprintf(stderr,"parameter 1 should be 3 byte company code :'%s'\n",        
       ppParms11#);                                                             
     abort();                                                                   
   }                                                                            
   memcpy(pProgParms->companyCode, ppParms11#, 3);                              
                                                                                
   if (strlen(ppParms12#) != 1)                                                 
   {                                                                            
     fprintf(stderr,"parameter 2 should be 1 byte version number :'%s'\n",      
       ppParms12#);                                                             
     abort();                                                                   
   }                                                                            
   pProgParms->oasisVersion = ppParms12#10#;                                    
                                                                                
   pProgParms->oflowMax = atoi(ppParms13#);                                     
   if (pProgParms->oflowMax < 2 * MAX_REC_SIZE)                                 
   {                                                                            
     fprintf(stderr,"overflow area size '%s' should be at least %d bytes\n",    
       ppParms13#, 2 * MAX_REC_SIZE);                                           
     abort();                                                                   
   }                                                                            
                                                                                
   /* store remaining optional parms in 'argc' and 'argv' stylee */             
                                                                                
   if (parmCount >= 5)                                                          
   {                                                                            
     pProgParms->optParmCount = parmCount - 4;                                  
     pProgParms->ppOptParms = &ppParms14#;                                      
                                                                                
     /* see if there's a FIRSTRUN  parameter */                                 
     pProgParms->isFirstRun = 0;                                                
     for(i=0; i<pProgParms->optParmCount && !pProgParms->isFirstRun; i++)       
     {                                                                          
       if(strcmp(pProgParms->ppOptParms1i#,"FIRSTRUN") == 0)                    
         pProgParms->isFirstRun = 1;                                            
     }                                                                          
   }                                                                            
                                                                                
   return;                                                                      
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : performInitialisation                             | */        
 /* |      - open files                                             | */        
 /* |      - read and store oasis parameters                        | */        
 /* |      - read and store company name                            | */        
 /* |      - read and store segment titles                          | */        
 /* |      - store dataset names                                    | */        
 /* |      - get run date from file or system date                  | */        
 /* |      - allocate area for overflows                            | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void performInitialisation(struct ProgramParameters* pProgParms,        
                                   FMS_PARM_STRUCT** ppFmsParms,                
                                   struct FilePointers* pFilePtrs,              
                                   struct ReportStatistics* pRepStats,          
                                   char** ppOverflowArea)                       
 {                                                                              
   time_t timer = 0;                                                            
   struct tm * pTime = NULL;                                                    
   fldata_t fileinfo;                                                           
   char companyVersion15# = "";                                                 
   char systemDDMMCCYY19# = "";                                                 
   char dateRec18# = "";                                                        
   register i=0;                                                                
   register j=0;                                                                
   short binaryDateLength = 3;                                                  
   char datecenRunType = 'B';                                                   
   int bespokeRoutineRunType = -1;                                              
   char dummyArea1MAX_REC_SIZE+2# = "";                                         
   struct RecordDetails dummyRec = {0};                                         
   struct RecordDetails* pDummyRec = NULL;                                      
                                                                                
   /* open files... */                                                          
                                                                                
   if ((pFilePtrs->pUpdFile = fopen("DD:UPDIN", "rb, type=record")) == NULL)    
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open update file\n");                       
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pMasFile = fopen("DD:MASIN", "rb, type=record")) == NULL)    
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open master file\n");                       
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pCompFile = fopen("DD:FMSCOMP","rb+,type=record")) == NULL)  
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open company file\n");                      
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pParmFile = fopen("DD:FMSPARM","rb+,type=record")) == NULL)  
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open parameter file\n");                    
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pMG64File = fopen("DD:MG64IN","rb,type=record")) == NULL)    
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open MG64 cards file\n");                   
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pMasOutFile = fopen("DD:MASOUT","wb,type=record")) == NULL)  
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open master out file\n");                   
     abort();                                                                   
   }                                                                            
                                                                                
   if ((pFilePtrs->pUpdRejFile = fopen("DD:REJUPD","wb,type=record")) == NULL)  
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open update reject file\n");                
     abort();                                                                   
   }                                                                            
                                                                                
   pFilePtrs->pDateFile = fopen("DD:DATEIN","rb,type=record");                  
                                                                                
   if ((pFilePtrs->pPrintFile = fopen("DD:MAINREP","wb,type=record")) == NULL)  
   {                                                                            
     fprintf(stderr,"ERROR - Cannot open print file\n");                        
     abort();                                                                   
   }                                                                            
                                                                                
   /* get company name from FMS company file */                                 
   readCompanyFile(pFilePtrs->pCompFile, pProgParms, pRepStats->companyName);   
   fclose(pFilePtrs->pCompFile);                                                
                                                                                
   /* store fms parameters with readparm function (gets a bit complex */        
   /* - see FMSLIB documetation for structure details)                */        
   memcpy(companyVersion, &(pProgParms->companyCode), 4);                       
   companyVersion14# = '\0';                                                    
   *ppFmsParms = readparm(companyVersion, pFilePtrs->pParmFile);                
   fclose(pFilePtrs->pParmFile);                                                
                                                                                
   /* if segment pointers are allowed to update each other, the results */      
   /* may be uNprEDicTabLe so make sure all pointers have code of 199   */      
   for(i=1; i<=(*ppFmsParms)->hNumSegments; i++)                                
   {                                                                            
     if((*ppFmsParms)->pSegments1i#.hSegLength > 0)                             
     {                                                                          
       if((*ppFmsParms)->pSegments1i#.pFields1REVPTR#.aUpdateAction != 199)     
       {                                                                        
         fprintf(stderr, "ERROR - rev ptr field on seg %02d "                   
           "should have update action = 199\n",i);                              
         abort();                                                               
       }                                                                        
       if((*ppFmsParms)->pSegments1i#.pFields1FWDPTR#.aUpdateAction != 199)     
       {                                                                        
         fprintf(stderr, "ERROR - fwd ptr field on seg %02d "                   
           "should have update action = 199\n",i);                              
         abort();                                                               
       }                                                                        
     }                                                                          
   }                                                                            
                                                                                
   /* use segment count to allocate space for segment report details */         
   pRepStats->pSegDetail = calloc                                               
     (((*ppFmsParms)->hNumSegments + 1), sizeof (struct SegmentDetails));       
   if(pRepStats->pSegDetail == NULL)                                            
   {                                                                            
     fprintf(stderr,"ERROR - unable to allocate seg details array\n");          
     abort();                                                                   
   }                                                                            
                                                                                
   /* get segment titles from 'MG64' cards */                                   
   readMG64cards(pFilePtrs->pMG64File,                                          
                 (*ppFmsParms)->hNumSegments,                                   
                 pProgParms->companyCode,                                       
                 pRepStats);                                                    
   fclose(pFilePtrs->pMG64File);                                                
                                                                                
   /* store dataset details for use in reports */                               
   strcpy(pRepStats->dataset10#.ddName, "UPDIN   ");                            
   strcpy(pRepStats->dataset10#.description, "Oasis update records     (in) "); 
   strcpy(pRepStats->dataset11#.ddName, "MASIN   ");                            
   strcpy(pRepStats->dataset11#.description, "Oasis master records     (in) "); 
   strcpy(pRepStats->dataset12#.ddName, "FMSCOMP ");                            
   strcpy(pRepStats->dataset12#.description, "Company File             (in) "); 
   strcpy(pRepStats->dataset13#.ddName, "FMSPARM ");                            
   strcpy(pRepStats->dataset13#.description, "Parameter File           (in) "); 
   strcpy(pRepStats->dataset14#.ddName, "MG64IN  ");                            
   strcpy(pRepStats->dataset14#.description, "MG64 Cards               (in) "); 
   strcpy(pRepStats->dataset15#.ddName, "DATEIN  ");                            
   strcpy(pRepStats->dataset15#.description, "Date override file       (in) "); 
   strcpy(pRepStats->dataset16#.ddName, "MASOUT  ");                            
   strcpy(pRepStats->dataset16#.description, "Updated database         (out)"); 
   strcpy(pRepStats->dataset17#.ddName, "REJUPD  ");                            
   strcpy(pRepStats->dataset17#.description, "Rejected updates         (out)"); 
   strcpy(pRepStats->dataset18#.ddName, "MAINREP ");                            
   strcpy(pRepStats->dataset18#.description, "Report print file        (out)"); 
                                                                                
   /* get system time and date and 'run date'... */                             
                                                                                
   /* try to open and read date override file */                                
   if(pFilePtrs->pDateFile != NULL)                                             
     fread((void *)&dateRec, 1, sizeof(dateRec), pFilePtrs->pDateFile);         
                                                                                
   /* get system time and date in character format */                           
   time(&timer);                                                                
   pTime = localtime(&timer);                                                   
   strftime(pRepStats->sysDate, 11, "%d/%m/%Y",pTime);                          
   strftime(pRepStats->sysTime, 9, "%I:%M %p",pTime);                           
                                                                                
   /* store experian binary run date - try date override first */               
   /* (datecen returns nulls if valid date not present)...     */               
                                                                                
   DATECEN(dateRec, pProgParms->experianRunDate,                                
     &datecenRunType, &binaryDateLength);                                       
                                                                                
   if(memcmp(pProgParms->experianRunDate, "\0\0", 3) == 0)                      
   {                                                                            
     /* override failed - use system date instead */                            
     strftime(systemDDMMCCYY, 9, "%d%m%Y",pTime);                               
     DATECEN(systemDDMMCCYY, pProgParms->experianRunDate,                       
       &datecenRunType, &binaryDateLength);                                     
                                                                                
     if(memcmp(pProgParms->experianRunDate, "\0\0", 3) == 0)                    
     {                                                                          
       fprintf(stderr,"ERROR - Cannot establish binary run date\n");            
       abort();                                                                 
     }                                                                          
     pProgParms->isDateOverridden = 0;                                          
     memcpy(pRepStats->runDate, pRepStats->sysDate, 10);                        
   }                                                                            
   else                                                                         
   {                                                                            
     /* date overriden - copy DDMMCCYY to DD/MM/CCYY for report */              
     pProgParms->isDateOverridden = 1;                                          
     for(i=0, j=0; i<8; i++)                                                    
     {                                                                          
       if(i==2 || i==4)                                                         
         pRepStats->runDate1j++# = '/';                                         
       pRepStats->runDate1j++# = dateRec1i#;                                    
     }                                                                          
   }                                                                            
                                                                                
   /* allocate overflow area - note that preceeding 2 bytes are reserved for */ 
   /* PL1 var strings in subroutines, even though they only work for < 32k   */ 
   *ppOverflowArea = malloc(pProgParms->oflowMax + 2);                          
   if(*ppOverflowArea == NULL)                                                  
   {                                                                            
     fprintf(stderr,"ERROR - Could not allocate memory for overflow area\n");   
     abort();                                                                   
   }                                                                            
   *ppOverflowArea += 2;                                                        
                                                                                
   /* set up dummy Oasis record object for bespoke routines */                  
   initialiseRecordObject(&dummyRec, *ppFmsParms, dummyArea+2, 'M');            
   pDummyRec = &dummyRec;                                                       
                                                                                
   /* initialise bespoke input routine */                                       
   GM40GPDI(&pProgParms,                                                        
            &pRepStats,                                                         
            &pDummyRec,                                                         
            ppFmsParms,                                                         
            &bespokeRoutineRunType);                                            
                                                                                
   /* initialise bespoke output routine */                                      
   GM40PDO(&pProgParms,                                                         
           &pRepStats,                                                          
           &pDummyRec,                                                          
           ppFmsParms,                                                          
           &bespokeRoutineRunType);                                             
                                                                                
   return;                                                                      
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : errorHandler                                      | */        
 /* |    - this gets called whenever abort() is called              | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void errorHandler(int SIG_TYPE)                                         
 {                                                                              
    perror(PROG_ID " program aborted ");                                        
    fclose(stderr);                                                             
    fclose(stdout);                                                             
    exit(4001);                                                                 
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : readCompanyFile                                   | */        
 /* |      - get company name from company file                     | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void readCompanyFile(FILE* pCompFile,                                   
                             struct ProgramParameters* pProgParms,              
                             char* companyName)                                 
 {                                                                              
   char tempStr1100# = "";                                                      
   char vsamKey110# = "";                                                       
   const size_t keyLen = 9;                                                     
                                                                                
   _Packed struct                                                               
   {                                                                            
      char  companyCode    13#;                                                 
      char  version;                                                            
      char  restOfKey      15#;                                                 
      char  compName       130#;                                                
      char  filler1        1150#;                                               
      char  currUrn        112#;                                                
      char  prevUrn        112#;                                                
      char  filler2        142#;                                                
      short fuzzyPassScore;                                                     
      char  filler3        188#;                                                
   }compRec;                                                                    
                                                                                
   /* set up key : comp + vers + 5 bytes high values */                         
   sprintf(vsamKey,"%3.3s%c%c%c%c%c%c",                                         
     pProgParms->companyCode, pProgParms->oasisVersion,                         
       0xFF,0xFF,0xFF,0xFF,0xFF);                                               
                                                                                
   /* locate record on VSAM */                                                  
   if (flocate(pCompFile,vsamKey,keyLen,__KEY_EQ) != 0)                         
   {                                                                            
     memcpy(tempStr,vsamKey,keyLen);                                            
     tempStr1keyLen# = '\0';                                                    
     fprintf(stderr,"company VSAM locate failed - key : '%s'\n",tempStr);       
     abort();                                                                   
   }                                                                            
                                                                                
   /* read record */                                                            
   if (0 == (fread(&compRec,1,sizeof(compRec),pCompFile)))                      
   {                                                                            
     fprintf(stderr,"VSAM read failed\n");                                      
     abort();                                                                   
   }                                                                            
                                                                                
   /* copy company name */                                                      
   memcpy(companyName, compRec.compName, sizeof(compRec.compName));             
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |  function : readMG64cards                                     | */        
 /* |      - get segment titles                                     | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void readMG64cards(FILE* pInFile,                                       
                           int numSegs,                                         
                           char* companyCode,                                   
                           struct ReportStatistics* pRepStats)                  
 {                                                                              
   register seg;                                                                
                                                                                
   struct                                                                       
   {                                                                            
     char cardID14#;                                                            
     char company13#;                                                           
     char fill1;                                                                
     char segNum12#;                                                            
     char fill2;                                                                
     char description130#;                                                      
   } mg64rec;                                                                   
                                                                                
                                                                                
   for(seg=0; seg<=numSegs; seg++)                                              
   {                                                                            
     fread(&mg64rec, 1, sizeof(mg64rec), pInFile);                              
     if(feof(pInFile))                                                          
     {                                                                          
       fprintf(stderr,"ERROR - MG64 card rec missing\n");                       
       abort();                                                                 
     }                                                                          
                                                                                
     mg64rec.fill2 = '\0'; /* makes segNum into C string for atoi function */   
                                                                                
     /* validate card */                                                        
     if(memcmp(mg64rec.cardID, "MG64", 4) != 0                                  
     || memcmp(mg64rec.company, companyCode, 3) != 0                            
     || atoi(mg64rec.segNum) != seg)                                            
     {                                                                          
       fprintf(stderr,"ERROR - expected MG64 card not found\n");                
       abort();                                                                 
     }                                                                          
                                                                                
     /* copy segment title */                                                   
     memcpy(pRepStats->pSegDetail1seg#.description,                             
            mg64rec.description,                                                
            sizeof(pRepStats->pSegDetail10#.description));                      
                                                                                
     /* convert case (funtion from stuff.h) */                                  
     titleCase(pRepStats->pSegDetail1seg#.description,                          
               sizeof(pRepStats->pSegDetail10#.description));                   
   }                                                                            
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : initialiseRecordObject                           | */        
 /* |     - initialise a new instance of RecordDetails object       | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void initialiseRecordObject(struct RecordDetails* pFmsRec,              
                                    FMS_PARM_STRUCT* pFmsParms,                 
                                    char* dataArea,                             
                                    char format)                                
 {                                                                              
   void* tempPtr;                                                               
   int urnField;                                                                
                                                                                
   /* set prefix length */                                                      
   switch(format)                                                               
   {                                                                            
     case 'U': pFmsRec->prefixLen = pFmsParms->hPrefixLength;                   
     break;                                                                     
     case 'M': pFmsRec->prefixLen = 0;                                          
     break;                                                                     
     default: /* format not valid! */                                           
     {                                                                          
       fprintf(stderr,"ERROR - bad call to 'initialiseRecordObject'\n");        
       abort();                                                                 
     }                                                                          
   }                                                                            
                                                                                
   if (pFmsParms->hUrnField < 1)                                                
   {                                                                            
     fprintf(stderr,"ERROR - urn field not set on parameter file!\n");          
     abort();                                                                   
   }                                                                            
                                                                                
   /* assign actual record data area to object */                               
   pFmsRec->pData = dataArea;                                                   
   pFmsRec->recLen = 0; /* empty record */                                      
                                                                                
   /* find location and length of urn */                                        
   urnField = pFmsParms->hUrnField;                                             
   pFmsRec->pUrn = dataArea + pFmsRec->prefixLen +                              
     pFmsParms->pSegments10#.pFields1urnField#.hStartPosition;                  
   pFmsRec->urnLen = pFmsParms->pSegments10#.pFields1urnField#.hFieldLength;    
                                                                                
   /* find location of urn suffix */                                            
   pFmsRec->pUrnSuffix = (short*)(dataArea + pFmsRec->prefixLen +               
     pFmsParms->pSegments10#.pFields1URN_SUFFIX#.hStartPosition);               
                                                                                
   /* find location of segment occurence counts */                              
   tempPtr = dataArea + pFmsRec->prefixLen +                                    
     pFmsParms->pSegments10#.hSegLength;                                        
   pFmsRec->pSegOcc = (short*) tempPtr;                                         
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : readOasisRecord                                  | */        
 /* |     - read record and update record object                    | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void readOasisRecord(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             FILE* pInFile,                                     
                             struct ReportStatistics* pRepStats,                
                             struct ProgramParameters* pProgParms,              
                             char* pOverflowArea)                               
 {                                                                              
   static char prevUrn1100#="";                                                 
   static char highUrn1100#="";                                                 
   register seg=0;                                                              
   static int bespokeRoutineRunType = 0;                                        
   static size_t sizeOfOverflowGroup = 0;                                       
                                                                                
   memcpy(prevUrn, pFmsRec->pUrn, pFmsRec->urnLen);                             
   pFmsRec->recLen = fread(pFmsRec->pData, 1, MAX_REC_SIZE, pInFile);           
                                                                                
   /* set high key if end of file */                                            
   if (feof(pInFile))                                                           
   {                                                                            
     memset(highUrn, 0xFF, pFmsRec->urnLen);                                    
     pFmsRec->pUrn = highUrn; /* nb: sets pointer, not data! */                 
     return;                                                                    
   }                                                                            
                                                                                
   /* populate overflow area if overflow group */                               
   if(*(pFmsRec->pUrnSuffix) > 0)                                               
   {                                                                            
     sizeOfOverflowGroup =                                                      
       readOverflow(pFmsRec, pFmsParms, pOverflowArea, pInFile);                
                                                                                
     pRepStats->countOvfRecsRead += sizeOfOverflowGroup - 1;                    
   }                                                                            
                                                                                
   /* check sequence */                                                         
   if (memcmp(prevUrn, pFmsRec->pUrn, pFmsRec->urnLen) > 0)                     
   {                                                                            
     fprintf(stderr,"ERROR - file out of sequence\n");                          
     abort();                                                                   
   }                                                                            
                                                                                
   /* check for duplicate masters */                                            
   if (pFmsRec->prefixLen == 0                                                  
   && memcmp(prevUrn, pFmsRec->pUrn, pFmsRec->urnLen) == 0)                     
   {                                                                            
     fprintf(stderr,"ERROR - master file contains duplicate urns\n");           
     prevUrn1pFmsRec->urnLen# = '\0';   /* convert to C string */               
     fprintf(stderr,"        URN : '%s'\n",prevUrn);                            
     abort();                                                                   
   }                                                                            
                                                                                
   /* set up segment offsets */                                                 
   pFmsRec->segOffset10# = pFmsRec->prefixLen;                                  
   pFmsRec->segOffset11# = pFmsRec->prefixLen +                                 
     pFmsParms->pSegments10#.hSegLength +                                       
       pFmsParms->hNumSegments * sizeof(short);                                 
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
   {                                                                            
     pFmsRec->segOffset1seg+1# = pFmsRec->segOffset1seg# +                      
       pFmsRec->pSegOcc1seg-1# * pFmsParms->pSegments1seg#.hSegLength;          
   }                                                                            
                                                                                
   /* increment report counts... */                                             
                                                                                
   if (pFmsRec->prefixLen == 0)                                                 
   {                                                                            
     pRepStats->countMasRecsRead++;                                             
     pRepStats->pSegDetail10#.mastersRead++;                                    
     for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                            
       pRepStats->pSegDetail1seg#.mastersRead += pFmsRec->pSegOcc1seg-1#;       
   }                                                                            
   else                                                                         
   {                                                                            
     pRepStats->countUpdRecsRead++;                                             
     pRepStats->pSegDetail10#.updatesRead++;                                    
     for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                            
       pRepStats->pSegDetail1seg#.updatesRead += pFmsRec->pSegOcc1seg-1#;       
   }                                                                            
                                                                                
   /* call bespoke input routine */                                             
   GM40GPDI(&pProgParms,                                                        
            &pRepStats,                                                         
            &pFmsRec,                                                           
            &pFmsParms,                                                         
            &bespokeRoutineRunType);                                            
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : readOverflow                                     | */        
 /* |     - copy records in overflow group into extended overflow   | */        
 /* |       area                                                    | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static size_t readOverflow(struct RecordDetails* pFmsRec,                      
                            FMS_PARM_STRUCT* pFmsParms,                         
                            char* pOverflowArea,                                
                            FILE* pInFile)                                      
 {                                                                              
   static char rec32k1MAX_REC_SIZE# = "";                                       
   static size_t recLen32k = 0;                                                 
   static char* pInData = NULL;                                                 
   static struct RecordDetails savedMasRec = {0};                               
   static size_t sizeOfOverflowGroup = 0;                                       
   static char* pOflowUrn = NULL;                                               
   static short* pOflowUrnSuffix = NULL;                                        
   register i;                                                                  
                                                                                
   sizeOfOverflowGroup = *(pFmsRec->pUrnSuffix);                                
                                                                                
   if(pFmsRec->prefixLen > 0)                                                   
   {                                                                            
     fprintf(stderr,"ERROR - update record reckons it's an overflow!\n");       
     abort();                                                                   
   }                                                                            
                                                                                
   /* set up record to receive the subsequent overflows... */                   
                                                                                
   /* pointer to drop segment 00 prefix */                                      
   pInData = rec32k + pFmsParms->pSegments10#.hSegLength;                       
                                                                                
   /* find location of urn on oflow rec */                                      
   pOflowUrn = rec32k +                                                         
     pFmsParms->pSegments10#.pFields1pFmsParms->hUrnField#.hStartPosition;      
                                                                                
   /* find location of urn suffix on oflow rec */                               
   pOflowUrnSuffix = (short*)(rec32k +                                          
     pFmsParms->pSegments10#.pFields1URN_SUFFIX#.hStartPosition);               
                                                                                
   /* set up overflow record... */                                              
                                                                                
   /* save current record data */                                               
   savedMasRec = *pFmsRec;                                                      
                                                                                
   /* initialise pointers and stuff for overflow area */                        
   initialiseRecordObject(pFmsRec, pFmsParms, pOverflowArea, 'M');              
   pFmsRec->isOverflow = 1;                                                     
                                                                                
   /* transfer stuff from saved record */                                       
   pFmsRec->recLen = savedMasRec.recLen;                                        
   memcpy(pFmsRec->pData, savedMasRec.pData, savedMasRec.recLen);               
   memcpy(pFmsRec->segOffset, savedMasRec.segOffset,                            
     sizeof(savedMasRec.segOffset));                                            
                                                                                
   /* read and store rest of overflow... */                                     
                                                                                
   for(i=sizeOfOverflowGroup+1; i<sizeOfOverflowGroup+sizeOfOverflowGroup; i++) 
   {                                                                            
     recLen32k = fread(rec32k, 1, MAX_REC_SIZE, pInFile);                       
 //printf("just read suffix %d length = %d\n",*pOflowUrnSuffix, recLen32k);     
                                                                                
     if(memcmp(pOflowUrn, pFmsRec->pUrn, pFmsRec->urnLen) != 0)                 
     {                                                                          
       fprintf(stderr,"ERROR - overflow group with multiple urns!\n");          
       abort();                                                                 
     }                                                                          
                                                                                
     if(*pOflowUrnSuffix != i)                                                  
     {                                                                          
       fprintf(stderr,"ERROR - overflow group urn suffix is wrong!\n");         
       abort();                                                                 
     }                                                                          
                                                                                
     /* concatenate data to end of overflow */                                  
     memcpy(pFmsRec->pData + pFmsRec->recLen,                                   
            pInData,                                                            
            recLen32k - (pInData - rec32k) );                                   
     pFmsRec->recLen += recLen32k - (pInData - rec32k);                         
   }                                                                            
                                                                                
   if (checkRecLength(pFmsRec, pFmsParms) != 0)                                 
   {                                                                            
     fprintf(stderr,"ERROR - overflow rec read is corrupt!\n");                 
     abort();                                                                   
   }                                                                            
                                                                                
   return sizeOfOverflowGroup;                                                  
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : checkRecLength                                   | */        
 /* |     - calculate length according to fms params and seg occs   | */        
 /* |       and compare with length stored in record object         | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static int checkRecLength(struct RecordDetails* pFmsRec,                       
                           FMS_PARM_STRUCT* pFmsParms)                          
 {                                                                              
   register seg=0;                                                              
   size_t   recLen=0;                                                           
                                                                                
   recLen = pFmsRec->prefixLen;                                                 
                                                                                
   recLen += pFmsParms->pSegments10#.hSegLength;                                
   recLen += pFmsParms->hNumSegments * sizeof(short);                           
                                                                                
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
     recLen += pFmsParms->pSegments1seg#.hSegLength * pFmsRec->pSegOcc1seg-1#;  
                                                                                
 //printf("rec len %d\n",pFmsRec->recLen);                                      
 //printf("calc rec len %d\n",recLen);                                          
   return(recLen - pFmsRec->recLen);                                            
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : compareUrns                                      | */        
 /* |     - compare urns on two record objects                      | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static int compareUrns(struct RecordDetails* pFmsRec1,                         
                        struct RecordDetails* pFmsRec2)                         
 {                                                                              
   return memcmp(pFmsRec1->pUrn, pFmsRec2->pUrn, pFmsRec1->urnLen);             
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : applyUpdateRecord                                | */        
 /* |     - apply update record to master record                    | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void applyUpdateRecord(struct RecordDetails* pUpdRec,                   
                               struct RecordDetails* pMasRec,                   
                               FMS_PARM_STRUCT* pFmsParms,                      
                               struct ProgramParameters* pProgParms,            
                               struct ReportStatistics* pRepStats)              
 {                                                                              
   register seg=0;                                                              
   register updOcc=0;                                                           
   static int masMatchOcc=0;                                                    
   static char* pMasSeg=NULL;                                                   
   static char* pUpdSeg=NULL;                                                   
   static size_t masSegOffset=0;                                                
                                                                                
   /*                                                                 */        
   /*  if the overflow flag is set on the *update* record then the    */        
   /*  master overflow might overflow! - abandon the update...        */        
   /*                                                                 */        
   if(pUpdRec->isOverflow)                                                      
   {                                                                            
     pUpdRec->isOverflow = 0;                                                   
     return;                                                                    
   }                                                                            
   /*                                                                 */        
   /*  if first update and no master then copy update to master       */        
   /*                                                                 */        
   if(pMasRec->recLen == 0)                                                     
   {                                                                            
 //printf(">>>create master from update\n");                                    
     memcpy(pMasRec->pData,                                                     
            pUpdRec->pData + pUpdRec->prefixLen,                                
            pUpdRec->recLen - pUpdRec->prefixLen);                              
     pMasRec->recLen = pUpdRec->recLen - pUpdRec->prefixLen;                    
                                                                                
     pRepStats->pSegDetail10#.segsAdded++;                                      
                                                                                
     /* fix offsets */                                                          
     pMasRec->segOffset10# = pMasRec->prefixLen;                                
     pMasRec->segOffset11# = pMasRec->prefixLen +                               
       pFmsParms->pSegments10#.hSegLength +                                     
         pFmsParms->hNumSegments * sizeof(short);                               
     for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                            
     {                                                                          
       pMasRec->segOffset1seg+1# = pMasRec->segOffset1seg# +                    
         pMasRec->pSegOcc1seg-1# * pFmsParms->pSegments1seg#.hSegLength;        
                                                                                
       pRepStats->pSegDetail1seg#.segsAdded += pMasRec->pSegOcc1seg-1#;         
     }                                                                          
                                                                                
     return;                                                                    
   }                                                                            
 //printf(">>>apply update record\n");                                          
   /*                                                                 */        
   /*  ensure update pointers do not clash with master                */        
   /*                                                                 */        
   fixClashingUpdatePointers(pUpdRec, pMasRec, pFmsParms);                      
 #if 0                                                                          
 //printf("POINTERS BEFORE UPDATE (AFTER FIX)\n");                              
 for(x=1; x<=pFmsParms->hNumSegments; x++)                                      
 for(y=1; y<=pMasRec->pSegOcc1x-1#; y++)                                        
 {                                                                              
 //printf("MASTER x%02d occ x%02d\n",x,y);                                      
 //printf("rev ptr"); showField(pMasRec, x, y, REVPTR, pFmsParms);              
 //printf("fwd ptr"); showField(pMasRec, x, y, FWDPTR, pFmsParms);              
 }                                                                              
 for(x=1; x<=pFmsParms->hNumSegments; x++)                                      
 for(y=1; y<=pUpdRec->pSegOcc1x-1#; y++)                                        
 {                                                                              
 //printf("UPDATE x%02d occ x%02d\n",x,y);                                      
 //printf("rev ptr"); showField(pUpdRec, x, y, REVPTR, pFmsParms);              
 //printf("fwd ptr"); showField(pUpdRec, x, y, FWDPTR, pFmsParms);              
 }                                                                              
 #endif                                                                         
   /*                                                                 */        
   /*  apply segment zero update                                      */        
   /*                                                                 */        
   seg = 0;                                                                     
   GM40GPDU(pFmsParms,                                                          
            seg,                                                                
            pUpdRec->pData + pUpdRec->prefixLen,                                
            pMasRec->pData,                                                     
            pProgParms->experianRunDate);                                       
   pRepStats->pSegDetail10#.segsUpdated++;                                      
                                                                                
   /*                                                                 */        
   /*  loop thru segs, match and update                               */        
   /*                                                                 */        
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
   {                                                                            
 //printf("   update seg %d  (occs:%d)\n",seg,pUpdRec->pSegOcc1seg-1#);         
     pUpdSeg = &(pUpdRec->pData1pUpdRec->segOffset1seg##);                      
     for(updOcc=1; updOcc<=pUpdRec->pSegOcc1seg-1#; updOcc++)                   
     {                                                                          
       masMatchOcc = GM40GPDK(pUpdSeg, seg, pMasRec, pFmsParms);                
 //printf("    mas match occ: %d\n",masMatchOcc);                               
       if (masMatchOcc > 0)                                                     
       {                                                                        
         masSegOffset = pMasRec->segOffset1seg# +                               
           pFmsParms->pSegments1seg#.hSegLength * (masMatchOcc - 1);            
                                                                                
         pMasSeg = pMasRec->pData  + masSegOffset;                              
                                                                                
         rewireLinkBoxPointers(seg,                                             
                               pMasSeg,                                         
                               pUpdSeg,                                         
                               pUpdRec,                                         
                               pMasRec,                                         
                               pFmsParms);                                      
                                                                                
         /* apply segment update */                                             
         GM40GPDU(pFmsParms,                                                    
                  seg,                                                          
                  pUpdSeg,                                                      
                  pMasSeg,                                                      
                  pProgParms->experianRunDate);                                 
                                                                                
         rewireUpdateSegPointers(seg,                                           
                                 pMasSeg,                                       
                                 pUpdSeg,                                       
                                 pUpdRec,                                       
                                 pMasRec,                                       
                                 pFmsParms);                                    
                                                                                
         pRepStats->pSegDetail1seg#.segsUpdated++;                              
       }                                                                        
       else                                                                     
       {                                                                        
 //printf("    adding segment: %d\n",seg);                                      
         addSegment(seg, pUpdSeg, pMasRec, pFmsParms);                          
                                                                                
         pRepStats->pSegDetail1seg#.segsAdded++;                                
       }                                                                        
                                                                                
       pUpdSeg += pFmsParms->pSegments1seg#.hSegLength;                         
     } /* loop thru upd occs */                                                 
   } /* loop thru seg types */                                                  
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : fixClashingUpdatePointers                        | */        
 /* |     - ensure update pointers do not clash with master         | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void fixClashingUpdatePointers(struct RecordDetails* pUpdRec,           
                                       struct RecordDetails* pMasRec,           
                                       FMS_PARM_STRUCT* pFmsParms)              
 {                                                                              
   register iSeg=0;                                                             
   register jSeg=0;                                                             
   register iOcc=0;                                                             
   register jOcc=0;                                                             
   static short highUpdPtr=0;                                                   
   static short highMasPtr=0;                                                   
   static int arePtrsReset=0;                                                   
   static size_t revPtrOffset=0;                                                
   static size_t fwdPtrOffset=0;                                                
   static char* pUpdFwdPtr=NULL;                                                
   static char* pUpdRevPtr=NULL;                                                
   static char* pMasFwdPtr=NULL;                                                
   static char* pUpdFwdPtr2=NULL;                                               
   static short storeUpdFwdPtr=0;                                               
                                                                                
   if (pMasRec->prefixLen != 0 || pUpdRec->prefixLen == 0)                      
   {                                                                            
     fprintf(stderr,"ERROR : incorrect 'fixClashingUpdatePointers' call\n ");   
     abort();                                                                   
   }                                                                            
                                                                                
   for(iSeg=1; iSeg<=pFmsParms->hNumSegments;  iSeg++)                          
   {                                                                            
     if(pMasRec->pSegOcc1iSeg-1# == 0 || pUpdRec->pSegOcc1iSeg-1# == 0)         
       continue;                                                                
                                                                                
     fwdPtrOffset = pFmsParms->pSegments1iSeg#.pFields1FWDPTR#.hStartPosition;  
     revPtrOffset = pFmsParms->pSegments1iSeg#.pFields1REVPTR#.hStartPosition;  
 /*                                                                   */        
 /*  store highest master pointer                                     */        
 /*                                                                   */        
     highMasPtr = 0;                                                            
     pMasFwdPtr = &(pMasRec->pData1pMasRec->segOffset1iSeg##) + fwdPtrOffset;   
     for(iOcc=1;  iOcc<=pMasRec->pSegOcc1iSeg-1#; iOcc++)                       
     {                                                                          
       if(*(short*)pMasFwdPtr > highMasPtr)                                     
         highMasPtr = *(short*)pMasFwdPtr;                                      
       pMasFwdPtr += pFmsParms->pSegments1iSeg#.hSegLength;                     
     }                                                                          
                                                                                
     if (highMasPtr == 0)                                                       
       continue;                                                                
 /*                                                                   */        
 /*  store highest update pointer                                     */        
 /*                                                                   */        
     highUpdPtr = 0;                                                            
     pUpdFwdPtr = &(pUpdRec->pData1pUpdRec->segOffset1iSeg##) + fwdPtrOffset;   
     for(iOcc=1;  iOcc<=pUpdRec->pSegOcc1iSeg-1#; iOcc++)                       
     {                                                                          
       if(*(short*)pUpdFwdPtr > highUpdPtr)                                     
         highUpdPtr = *(short*)pUpdFwdPtr;                                      
       pUpdFwdPtr += pFmsParms->pSegments1iSeg#.hSegLength;                     
     }                                                                          
                                                                                
     highUpdPtr += highMasPtr;                                                  
     highMasPtr++;                                                              
 /*                                                                   */        
 /*  loop through update forward pointers                             */        
 /*                                                                   */        
     pUpdFwdPtr = &(pUpdRec->pData1pUpdRec->segOffset1iSeg##) + fwdPtrOffset;   
     for(iOcc=1;  iOcc<=pUpdRec->pSegOcc1iSeg-1#; iOcc++)                       
     {                                                                          
       if(*(short*)pUpdFwdPtr >= highMasPtr)                                    
       {                                                                        
         pUpdFwdPtr += pFmsParms->pSegments1iSeg#.hSegLength;                   
         continue;                                                              
       }                                                                        
                                                                                
       highUpdPtr++;                                                            
       storeUpdFwdPtr = *(short*)pUpdFwdPtr;                                    
       arePtrsReset = 0;                                                        
                                                                                
       switch(pFmsParms->pSegments1iSeg#.hForwardLink)                          
       {                                                                        
         case 0 : /* no link - do nowt */                                       
         break;                                                                 
                                                                                
         case -1 : /* many forward linked segs */                               
                                                                                
           for(jSeg=iSeg; jSeg<=pFmsParms->hNumSegments;  jSeg++)               
           {                                                                    
             if(pFmsParms->pSegments1jSeg#.hReverseLink != iSeg)                
               continue;                                                        
                                                                                
             pUpdRevPtr = &(pUpdRec->pData1pUpdRec->segOffset1jSeg##) +         
               revPtrOffset;                                                    
             for(jOcc=1;  jOcc<=pUpdRec->pSegOcc1jSeg-1#; jOcc++)               
             {                                                                  
               if(*(short*)pUpdRevPtr == *(short*)pUpdFwdPtr)                   
               {                                                                
                 arePtrsReset = 1;                                              
                 *(short*)pUpdRevPtr = highUpdPtr;                              
               }                                                                
               pUpdRevPtr += pFmsParms->pSegments1jSeg#.hSegLength;             
             }                                                                  
           }                                                                    
                                                                                
         break;                                                                 
                                                                                
         default: /* one to one link */                                         
                                                                                
           jSeg = pFmsParms->pSegments1iSeg#.hForwardLink;                      
                                                                                
           pUpdRevPtr = &(pUpdRec->pData1pUpdRec->segOffset1jSeg##) +           
             revPtrOffset;                                                      
           for(jOcc=1;  jOcc<=pUpdRec->pSegOcc1jSeg-1#; jOcc++)                 
           {                                                                    
             if(*(short*)pUpdRevPtr == *(short*)pUpdFwdPtr)                     
             {                                                                  
               arePtrsReset = 1;                                                
               *(short*)pUpdRevPtr = highUpdPtr;                                
             }                                                                  
             pUpdRevPtr += pFmsParms->pSegments1jSeg#.hSegLength;               
           }                                                                    
                                                                                
       } /* end of switch */                                                    
                                                                                
       /* loop through and change any other fwd ptrs that link to */            
       /* reverse pointers just reset...                          */            
                                                                                
       if(arePtrsReset)                                                         
       {                                                                        
         pUpdFwdPtr2 =                                                          
           &(pUpdRec->pData1pUpdRec->segOffset1iSeg##) + fwdPtrOffset;          
         for(jOcc=1;  jOcc<=pUpdRec->pSegOcc1iSeg-1#; jOcc++)                   
         {                                                                      
           if(*(short*)pUpdFwdPtr2 == storeUpdFwdPtr)                           
             *(short*)pUpdFwdPtr2 = highUpdPtr;                                 
                                                                                
           pUpdFwdPtr2 += pFmsParms->pSegments1iSeg#.hSegLength;                
         }                                                                      
       }                                                                        
                                                                                
       pUpdFwdPtr += pFmsParms->pSegments1iSeg#.hSegLength;                     
     }  /* end loop through update forward pointers */                          
                                                                                
                                                                                
   }  /* end loop thru seg types */                                             
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : rewireLinkBoxPointers                            | */        
 /* |     - maintain links between link boxes before applying       | */        
 /* |       update                                                  | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void rewireLinkBoxPointers(int segment,                                 
                                   char* pMasSeg,                               
                                   char* pUpdSeg,                               
                                   struct RecordDetails* pUpdRec,               
                                   struct RecordDetails* pMasRec,               
                                   FMS_PARM_STRUCT* pFmsParms)                  
 {                                                                              
   static int revSeg=0;                                                         
   static int revPtrOffset=0;                                                   
   static int fwdPtrOffset=0;                                                   
   static char* pMasFwdPtr=NULL;                                                
   static char* pMasRevPtr=NULL;                                                
   static char* pUpdRevPtr=NULL;                                                
   static int occ=0;                                                            
                                                                                
   revSeg = pFmsParms->pSegments1segment#.hReverseLink;                         
                                                                                
   /* if segment not child of link box then do nothing */                       
   if (pFmsParms->pSegments1revSeg#.aType10# != 'L')                            
     return;                                                                    
                                                                                
   revPtrOffset =                                                               
     pFmsParms->pSegments1segment#.pFields1REVPTR#.hStartPosition;              
   fwdPtrOffset =                                                               
     pFmsParms->pSegments1revSeg#.pFields1FWDPTR#.hStartPosition;               
                                                                                
   pMasRevPtr = pMasSeg + revPtrOffset;                                         
   pUpdRevPtr = pUpdSeg + revPtrOffset;                                         
                                                                                
   pMasFwdPtr = &(pMasRec->pData1pMasRec->segOffset1revSeg##) + fwdPtrOffset;   
                                                                                
   for(occ=1; occ<=pMasRec->pSegOcc1revSeg-1#; occ++)                           
   {                                                                            
     /* connect link box to master seg about to be updated */                   
     if (*(short*)pMasFwdPtr == *(short*)pUpdRevPtr)                            
       *(short*)pMasFwdPtr = *(short*)pMasRevPtr;                               
                                                                                
     pMasFwdPtr += pFmsParms->pSegments1revSeg#.hSegLength;                     
   } /* loop thru occs */                                                       
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : rewireUpdateSegPointers                          | */        
 /* |     - reset pointers on master from matched, applied update   | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void rewireUpdateSegPointers(int parentSeg,                             
                                     char* pMasParentSeg,                       
                                     char* pUpdParentSeg,                       
                                     struct RecordDetails* pUpdRec,             
                                     struct RecordDetails* pMasRec,             
                                     FMS_PARM_STRUCT* pFmsParms)                
 {                                                                              
   register childSeg=0;                                                         
   register occ=0;                                                              
   static size_t parentFwdPtrOffset=0;                                          
   static size_t parentRevPtrOffset=0;                                          
   static size_t childFwdPtrOffset=0;                                           
   static size_t childRevPtrOffset=0;                                           
   static char* pUpdChildSeg=NULL;                                              
   static char* pMasFwdPtr=NULL;                                                
   static char* pMasRevPtr=NULL;                                                
   static char* pUpdParentFwdPtr=NULL;                                          
   static char* pUpdParentRevPtr=NULL;                                          
   static char* pUpdChildRevPtr=NULL;                                           
   static int revSeg=0;                                                         
   static char* pMasRevSegFwdPtr=NULL;                                          
                                                                                
   parentFwdPtrOffset =                                                         
     pFmsParms->pSegments1parentSeg#.pFields1FWDPTR#.hStartPosition;            
   parentRevPtrOffset =                                                         
     pFmsParms->pSegments1parentSeg#.pFields1REVPTR#.hStartPosition;            
                                                                                
   pMasFwdPtr = pMasParentSeg + parentFwdPtrOffset;                             
   pMasRevPtr = pMasParentSeg + parentRevPtrOffset;                             
                                                                                
   pUpdParentFwdPtr = pUpdParentSeg + parentFwdPtrOffset;                       
   pUpdParentRevPtr = pUpdParentSeg + parentRevPtrOffset;                       
                                                                                
                                                                                
   for(childSeg=parentSeg+1; childSeg<=pFmsParms->hNumSegments; childSeg++)     
   {                                                                            
     if(pFmsParms->pSegments1childSeg#.hReverseLink != parentSeg)               
       continue;                                                                
                                                                                
     childFwdPtrOffset =                                                        
       pFmsParms->pSegments1childSeg#.pFields1FWDPTR#.hStartPosition;           
     childRevPtrOffset =                                                        
       pFmsParms->pSegments1childSeg#.pFields1REVPTR#.hStartPosition;           
                                                                                
     pUpdChildSeg = &(pUpdRec->pData1pUpdRec->segOffset1childSeg##);            
     for(occ=1;  occ<=pUpdRec->pSegOcc1childSeg-1#; occ++)                      
     {                                                                          
       pUpdChildRevPtr = pUpdChildSeg + childRevPtrOffset;                      
                                                                                
       if (*(short*)pMasFwdPtr == 0)                                            
         *(short*)pMasFwdPtr = *(short*)pUpdParentFwdPtr;                       
       else                                                                     
         if (*(short*)pUpdParentFwdPtr == *(short*)pUpdChildRevPtr)             
           *(short*)pUpdChildRevPtr = *(short*)pMasFwdPtr;                      
                                                                                
       pUpdChildSeg += pFmsParms->pSegments1childSeg#.hSegLength;               
                                                                                
     } /* loop thru occs */                                                     
   } /* loop thru seg types */                                                  
                                                                                
   /* reset fwd pointer of parent of current matched update */                  
                                                                                
   revSeg = pFmsParms->pSegments1parentSeg#.hReverseLink;                       
   if(revSeg != 0)                                                              
   {                                                                            
     pMasParentSeg = &(pMasRec->pData1pMasRec->segOffset1revSeg##);             
                                                                                
     parentRevPtrOffset =                                                       
       pFmsParms->pSegments1revSeg#.pFields1REVPTR#.hStartPosition;             
                                                                                
     for(occ=1;  occ<=pMasRec->pSegOcc1revSeg-1#; occ++)                        
     {                                                                          
       pMasRevSegFwdPtr = pMasParentSeg + parentRevPtrOffset;                   
                                                                                
       if (*(short*)pMasRevSegFwdPtr == *(short*)pUpdParentRevPtr)              
         *(short*)pMasRevSegFwdPtr = *(short*)pMasRevPtr;                       
                                                                                
       pMasParentSeg += pFmsParms->pSegments1revSeg#.hSegLength;                
                                                                                
     } /* loop thru occs */                                                     
   } /* if parent has parent (other than 00) */                                 
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : addSegment                                       | */        
 /* |     - shoehorn update segment into master record              | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void addSegment(int segNum,                                             
                        char* pUpdSeg,                                          
                        struct RecordDetails* pMasRec,                          
                        FMS_PARM_STRUCT* pFmsParms)                             
 {                                                                              
   static size_t recSplitOffset=0;                                              
   static size_t sizeOfMovingChunk=0;                                           
   register i=0;                                                                
                                                                                
   recSplitOffset = pMasRec->segOffset1segNum# +                                
     pFmsParms->pSegments1segNum#.hSegLength * pMasRec->pSegOcc1segNum-1#;      
                                                                                
   sizeOfMovingChunk = pMasRec->recLen - recSplitOffset;                        
                                                                                
   /* make space on master */                                                   
   memmove(pMasRec->pData + recSplitOffset +                                    
             pFmsParms->pSegments1segNum#.hSegLength,                           
           pMasRec->pData + recSplitOffset,                                     
           sizeOfMovingChunk);                                                  
                                                                                
   /* copy segment */                                                           
   memcpy(pMasRec->pData + recSplitOffset,                                      
          pUpdSeg,                                                              
          pFmsParms->pSegments1segNum#.hSegLength);                             
                                                                                
   /* reset record stats */                                                     
   pMasRec->pSegOcc1segNum-1#++;                                                
   pMasRec->recLen += pFmsParms->pSegments1segNum#.hSegLength;                  
   for(i=segNum+1; i<=pFmsParms->hNumSegments; i++)                             
   {                                                                            
     pMasRec->segOffset1i# += pFmsParms->pSegments1segNum#.hSegLength;          
   }                                                                            
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : checkForPossibleOverflow                         | */        
 /* |     - see if update might overflow storage area and take      | */        
 /* |       appropriate action                                      | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void checkForPossibleOverflow(struct RecordDetails* pUpdRec,            
                                      struct RecordDetails* pMasRec,            
                                      FMS_PARM_STRUCT* pFmsParms,               
                                      struct ProgramParameters* pProgParms,     
                                      struct ReportStatistics* pRepStats,       
                                      FILE* pUpdRejFile,                        
                                      char* pOverflowArea)                      
 {                                                                              
   static int possibleRecLength;                                                
   static struct RecordDetails savedMasRec;                                     
                                                                                
 //printf("## master rec len so far: %d\n",pMasRec->recLen);                    
 //printf("## update rec len: %d\n",pUpdRec->recLen);                           
   possibleRecLength =                                                          
     pMasRec->recLen + pUpdRec->recLen -  pUpdRec->segOffset11#;                
 //printf("## calculated possible rec len: %d\n",possibleRecLength);            
                                                                                
   /* if no overflow expected, return zero */                                   
   if (!pMasRec->isOverflow && possibleRecLength < MAX_REC_SIZE)                
     return;                                                                    
                                                                                
   if (pMasRec->isOverflow && possibleRecLength < pProgParms->oflowMax)         
     return;                                                                    
                                                                                
   /* if overflow area is going to overflow, reject the update record */        
   if (pMasRec->isOverflow)                                                     
   {                                                                            
     fwrite(pUpdRec->pData,1,pUpdRec->recLen, pUpdRejFile);                     
     pRepStats->countRejRecsWrit++;                                             
     pUpdRec->isOverflow = 1;                                                   
     return;                                                                    
   }                                                                            
                                                                                
   /* set up overflow record... */                                              
 //printf("### SETUP NEW OVERFLOW\n");                                          
                                                                                
   /* save current record data */                                               
   savedMasRec = *pMasRec;                                                      
                                                                                
   /* initialise pointers and stuff for overflow area */                        
   initialiseRecordObject(pMasRec, pFmsParms, pOverflowArea, 'M');              
   pMasRec->isOverflow = 1;                                                     
                                                                                
   /* transfer stuff from saved record */                                       
   pMasRec->recLen = savedMasRec.recLen;                                        
   memcpy(pMasRec->pData, savedMasRec.pData, savedMasRec.recLen);               
   memcpy(pMasRec->segOffset, savedMasRec.segOffset,                            
     sizeof(savedMasRec.segOffset));                                            
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : writeOasisRecord                                 | */        
 /* |     - go on, guess                                            | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void writeOasisRecord(struct RecordDetails* pFmsRec,                    
                              FMS_PARM_STRUCT* pFmsParms,                       
                              FILE* pOutFile,                                   
                              struct ReportStatistics* pRepStats,               
                              struct ProgramParameters* pProgParms)             
 {                                                                              
   register seg=0;                                                              
   static int bespokeRoutineRunType = 0;                                        
   static size_t sizeOfOverflowGroup;                                           
                                                                                
   dedupeLinkBoxes(pFmsRec, pFmsParms, pRepStats);                              
                                                                                
   /* increment report counts (recs written excludes overflows) */              
   pRepStats->countMasRecsWrit++;                                               
   pRepStats->pSegDetail10#.segsWritten++;                                      
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
     pRepStats->pSegDetail1seg#.segsWritten += pFmsRec->pSegOcc1seg-1#;         
                                                                                
   /* call bespoke output routine */                                            
   GM40PDO(&pProgParms,                                                         
           &pRepStats,                                                          
           &pFmsRec,                                                            
           &pFmsParms,                                                          
           &bespokeRoutineRunType);                                             
                                                                                
   if(pFmsRec->isOverflow)                                                      
   {                                                                            
     sizeOfOverflowGroup = writeOverflow(pFmsRec, pFmsParms, pOutFile);         
                                                                                
     pRepStats->countOvfRecsWrit += sizeOfOverflowGroup - 1;                    
                                                                                
     if (pFmsRec->recLen > pRepStats->sizeOfLargestOvfRecord)                   
       pRepStats->sizeOfLargestOvfRecord = pFmsRec->recLen;                     
                                                                                
     if (sizeOfOverflowGroup > pRepStats->sizeOfLargestOvfGroup)                
       pRepStats->sizeOfLargestOvfGroup = sizeOfOverflowGroup;                  
                                                                                
     pRepStats->countOverflows++;                                               
   }                                                                            
   else                                                                         
   {                                                                            
     fwrite(pFmsRec->pData,1,pFmsRec->recLen, pOutFile);                        
   }                                                                            
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : dedupeLinkBoxes                                  | */        
 /* |     - remove duplicate link boxes                             | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void dedupeLinkBoxes(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             struct ReportStatistics* pRepStats)                
 {                                                                              
   register seg=0;                                                              
   register occ1=0;                                                             
   register occ2=0;                                                             
   register i=0;                                                                
   static char* pSeg1=NULL;                                                     
   static char* pSeg2=NULL;                                                     
   static size_t sizeOfMovingChunk=0;                                           
                                                                                
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
   {                                                                            
     if (pFmsParms->pSegments1seg#.aType10# != 'L')                             
       continue;                                                                
                                                                                
     pSeg1 = &(pFmsRec->pData1pFmsRec->segOffset1seg##);                        
                                                                                
     for(occ1=1;  occ1<=pFmsRec->pSegOcc1seg-1#; occ1++)                        
     {                                                                          
       pSeg2 = pSeg1 + pFmsParms->pSegments1seg#.hSegLength;                    
                                                                                
       for(occ2=occ1+1;  occ2<=pFmsRec->pSegOcc1seg-1#;)                        
       {                                                                        
         if(memcmp(pSeg1, pSeg2, pFmsParms->pSegments1seg#.hSegLength) == 0)    
         {                                                                      
           /* chop out segment */                                               
           sizeOfMovingChunk = pFmsRec->recLen - (pSeg2 - pFmsRec->pData) -     
             pFmsParms->pSegments1seg#.hSegLength;                              
           memmove(pSeg2, pSeg2 +  pFmsParms->pSegments1seg#.hSegLength,        
             sizeOfMovingChunk);                                                
                                                                                
           /* adjust record object statistics */                                
           pFmsRec->pSegOcc1seg-1#--;  /* note: this will shorten occ loops */  
           pFmsRec->recLen -= pFmsParms->pSegments1seg#.hSegLength;             
           for(i=seg+1; i<=pFmsParms->hNumSegments; i++)                        
           {                                                                    
             pFmsRec->segOffset1i# -= pFmsParms->pSegments1seg#.hSegLength;     
           }                                                                    
                                                                                
           /* adjust report counts */                                           
           pRepStats->pSegDetail1seg#.segsAdded--;                              
           pRepStats->pSegDetail1seg#.segsUpdated++;                            
         }                                                                      
                                                                                
         else                                                                   
         /* no need to increment seg pointer if seg deleted */                  
         {                                                                      
           pSeg2 += pFmsParms->pSegments1seg#.hSegLength;                       
           occ2++;                                                              
         }                                                                      
       } /* loop thru occs */                                                   
                                                                                
       pSeg1 += pFmsParms->pSegments1seg#.hSegLength;                           
     } /* loop thru occs */                                                     
                                                                                
   } /* loop thru seg types */                                                  
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : writeOverflow                                    | */        
 /* |     - chop overflow to bits and write with urn suffixes       | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static size_t writeOverflow(struct RecordDetails* pFmsRec,                     
                             FMS_PARM_STRUCT* pFmsParms,                        
                             FILE* pOutFile)                                    
 {                                                                              
   char rec32k1MAX_REC_SIZE#;                                                   
   char* pOut;                                                                  
   char* pChunk;                                                                
   size_t lenChunk;                                                             
   size_t numOverflowRecs;                                                      
   short urnSuffix;                                                             
   short* pOutUrnSuffix=NULL;                                                   
   register seg;                                                                
   register occ;                                                                
                                                                                
   /* count how many overflow recs required... */                               
                                                                                
   /* each overflow record prefixed with seg 00 minus counts but first */       
   /* record *will* have counts                                        */       
                                                                                
   lenChunk = pFmsParms->pSegments10#.hSegLength +                              
     (pFmsParms->hNumSegments * sizeof(short));                                 
                                                                                
   numOverflowRecs=1;                                                           
                                                                                
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
   {                                                                            
     for(occ=1; occ<=pFmsRec->pSegOcc1seg-1#; occ++)                            
     {                                                                          
 //intf("seg %02d occ %d \n",seg,occ);                                          
 //printf("size of chunk so far %d\n",lenChunk);                                
       if(lenChunk + pFmsParms->pSegments1seg#.hSegLength >= MAX_REC_SIZE)      
       {                                                                        
         numOverflowRecs++;                                                     
         lenChunk = pFmsParms->pSegments10#.hSegLength +                        
           pFmsParms->pSegments1seg#.hSegLength;                                
 //printf("new chunk\n");                                                       
       }                                                                        
       else                                                                     
         lenChunk += pFmsParms->pSegments1seg#.hSegLength;                      
     }                                                                          
   }                                                                            
 //printf("need %d ovf recs \n",numOverflowRecs);                               
                                                                                
   /* note: urn suffix starts with number of overflows, n (not 1 as with      */
   /*       traditional overflows) and goes up to n+n-1.  this will let the   */
   /*       program know the size of the group after reading the first record */
                                                                                
   /* build and write the overflow records... */                                
                                                                                
   lenChunk = pFmsParms->pSegments10#.hSegLength +                              
     (pFmsParms->hNumSegments * sizeof(short));                                 
                                                                                
   pChunk = pFmsRec->pData;                                                     
   urnSuffix = numOverflowRecs;                                                 
   pOut = rec32k;                                                               
   pOutUrnSuffix =                                                              
     (short*)(rec32k + ((char*)(pFmsRec->pUrnSuffix) - pFmsRec->pData));        
 //rintf("urnSuff - urn \n",(char*)(pFmsRec->pUrnSuffix) - pFmsRec->pUrn);      
 //printf("urnSuff - urn \n",(char*)(pFmsRec->pUrnSuffix) - pFmsRec->pUrn);     
                                                                                
   for(seg=1; seg<=pFmsParms->hNumSegments; seg++)                              
   {                                                                            
     for(occ=1; occ<=pFmsRec->pSegOcc1seg-1#; occ++)                            
     {                                                                          
       if(lenChunk + pFmsParms->pSegments1seg#.hSegLength >= MAX_REC_SIZE)      
       {                                                                        
         if(urnSuffix == numOverflowRecs) /* first rec */                       
         {                                                                      
           memcpy(pOut, pChunk, lenChunk);                                      
           pOut = rec32k + pFmsParms->pSegments10#.hSegLength;                  
           pChunk += lenChunk;                                                  
         }                                                                      
         else                                                                   
         {                                                                      
           memcpy(pOut, pChunk, lenChunk - pFmsParms->pSegments10#.hSegLength); 
           pChunk += lenChunk - pFmsParms->pSegments10#.hSegLength;             
         }                                                                      
         *pOutUrnSuffix = urnSuffix;                                            
         fwrite(rec32k, 1, lenChunk, pOutFile);                                 
         lenChunk = pFmsParms->pSegments10#.hSegLength +                        
           pFmsParms->pSegments1seg#.hSegLength;                                
         urnSuffix++;                                                           
       }                                                                        
       else                                                                     
         lenChunk += pFmsParms->pSegments1seg#.hSegLength;                      
     }                                                                          
   }                                                                            
                                                                                
   /* write last chunk */                                                       
   memcpy(pOut, pChunk, lenChunk - pFmsParms->pSegments10#.hSegLength);         
   *pOutUrnSuffix = urnSuffix;                                                  
   fwrite(rec32k, 1, lenChunk, pOutFile);                                       
                                                                                
   return numOverflowRecs;                                                      
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : performTermination                               | */        
 /* |     - call optional routines and print reports                | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void performTermination(struct ReportStatistics* pRepStats,             
                                struct ProgramParameters* pProgParms,           
                                struct FilePointers* pFilePtrs,                 
                                FMS_PARM_STRUCT* pFmsParms)                     
 {                                                                              
   int bespokeRoutineRunType = 1;                                               
   char dummyArea1MAX_REC_SIZE+2# = "";                                         
   struct RecordDetails dummyRec = {0};                                         
   struct RecordDetails* pDummyRec = NULL;                                      
                                                                                
   printReports(pRepStats, pProgParms, pFilePtrs, pFmsParms);                   
                                                                                
   /* set up dummy record for bespoke routines */                               
   initialiseRecordObject(&dummyRec, pFmsParms, dummyArea+2, 'M');              
   pDummyRec = &dummyRec;                                                       
                                                                                
   /* run termination routine for bespoke input routine */                      
   GM40GPDI(&pProgParms,                                                        
            &pRepStats,                                                         
            &pDummyRec,                                                         
            &pFmsParms,                                                         
            &bespokeRoutineRunType);                                            
                                                                                
   /* run termination routine for bespoke output routine */                     
   GM40PDO(&pProgParms,                                                         
           &pRepStats,                                                          
           &pDummyRec,                                                          
           &pFmsParms,                                                          
           &bespokeRoutineRunType);                                             
                                                                                
 }                                                                              
 /*  _______________________________________________________________  */        
 /* |                                                               | */        
 /* |   function : printReports                                     | */        
 /* |     - print reports (funnily enough)                          | */        
 /* |_______________________________________________________________| */        
 /*                                                                   */        
 static void printReports(struct ReportStatistics* pRepStats,                   
                          struct ProgramParameters* pProgParms,                 
                          struct FilePointers* pFilePtrs,                       
                          FMS_PARM_STRUCT* pFmsParms)                           
 {                                                                              
   register i = 0;                                                              
   char text1133# = "";                                                         
   char tempStr120# = "";                                                       
   char spoolNum13# = "01";                                                     
   char asaChar = ' ';                                                          
   char dsname145# = "";                                                        
   char volSer19# = "";                                                         
                                                                                
   /* print header */                                                           
                                                                                
   sprintf(text,"%-30.30s%-30.30s%-40.40s%-9.9s%-14.14s%-5.5s%4d",              
     "Experian",pRepStats->companyName,"",                                      
       "Run Date",pRepStats->sysDate,"Page",1);                                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'1',text);                  
                                                                                
   sprintf(text,"%-9.9s%2.2s%-19.19s%-70.70s%-9.9s%-8.8s%15.15s",               
     PROG_ID"-",spoolNum,"","Oasis Master File Update - Processing Summary",    
       "Run Time",pRepStats->sysTime,"");                                       
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   memset(text, '_', 132);                                                      
   text1132# = '\0';                                                            
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-132.132s","Program Parameters");                             
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
   sprintf(text,"%-132.132s","__________________");                             
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-5.5s%-50.50s%-3.3s%-74.74s","",                              
     "Oasis company code",pProgParms->companyCode,"");                          
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
                                                                                
   sprintf(text,"%-5.5s%-50.50s%-77c","",                                       
     "Oasis version",pProgParms->oasisVersion);                                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-5.5s%-50.50s%-77d","",                                       
     "Max overflow area size",pProgParms->oflowMax);                            
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   if(pProgParms->isDateOverridden)                                             
     sprintf(text,"%-5.5s%-50.50s%-10.10s%-67.67s","",                          
       "Run Date",pRepStats->runDate," (date overridden)");                     
   else                                                                         
     sprintf(text,"%-5.5s%-50.50s%-10.10s%-67.67s","",                          
       "Run Date",pRepStats->runDate," (system date used)");                    
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-132.132s","Segment Counts");                                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
                                                                                
   sprintf(text,"%-36.36s%13.13s%13.13s%13.13s%13.13s%13.13s%2.2s%-29.29s",     
     "______________","Updates","Masters","New Segs","Segments","Segments","",  
      "Reconciliation");                                                        
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-36.36s%13.13s%13.13s%13.13s%13.13s%13.13s%2.2s%-29.29s",     
     "","Read","Read","Added","Updated","Written","","Check");                  
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   for(i=0; i<=pFmsParms->hNumSegments; i++)                                    
   {                                                                            
     if (i==0)                                                                  
       asaChar = '0';                                                           
     else                                                                       
       asaChar = ' ';                                                           
                                                                                
     sprintf(text,"%-5.5s%02d%-1.1s%-30.30s",                                   
     "",i,"",pRepStats->pSegDetail1i#.description);                             
     intToPic(tempStr, pRepStats->pSegDetail1i#.updatesRead, "ZZZ,ZZZ,ZZ9BB");  
     strcat(text, tempStr);                                                     
     intToPic(tempStr, pRepStats->pSegDetail1i#.mastersRead, "ZZZ,ZZZ,ZZ9BB");  
     strcat(text, tempStr);                                                     
     intToPic(tempStr, pRepStats->pSegDetail1i#.segsAdded, "ZZZ,ZZZ,ZZ9BB");    
     strcat(text, tempStr);                                                     
     intToPic(tempStr, pRepStats->pSegDetail1i#.segsUpdated, "ZZZ,ZZZ,ZZ9BB");  
     strcat(text, tempStr);                                                     
     intToPic(tempStr, pRepStats->pSegDetail1i#.segsWritten, "ZZZ,ZZZ,ZZ9BB");  
     strcat(text, tempStr);                                                     
                                                                                
     /* reconciliation */                                                       
     if((pRepStats->pSegDetail1i#.mastersRead +                                 
         pRepStats->pSegDetail1i#.segsAdded   ==                                
         pRepStats->pSegDetail1i#.segsWritten) &&                               
        (pRepStats->pSegDetail1i#.segsAdded   +                                 
         pRepStats->pSegDetail1i#.segsUpdated ==                                
         pRepStats->pSegDetail1i#.updatesRead))                                 
       strcat(text,"reconciled okay              ");                            
     else                                                                       
       strcat(text,"ERROR!! - does not reconcile ");                            
                                                                                
     printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,asaChar,text);            
   }                                                                            
                                                                                
   if (pRepStats->countRejRecsWrit != 0)                                        
   {                                                                            
     sprintf(text,"%-5.5s%-127.127s","","*** WARNING : "                        
       "Updates were REJECTED because the overflow area overflowed!");          
     printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                
   }                                                                            
                                                                                
   /* print header */                                                           
                                                                                
   sprintf(text,"%-30.30s%-30.30s%-40.40s%-9.9s%-14.14s%-5.5s%4d",              
     "Experian",pRepStats->companyName,"",                                      
       "Run Date",pRepStats->sysDate,"Page",2);                                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'1',text);                  
                                                                                
   sprintf(text,"%-9.9s%2.2s%-19.19s%-70.70s%-9.9s%-8.8s%15.15s",               
     PROG_ID"-",spoolNum,"","Oasis Master File Update - Processing Summary",    
       "Run Time",pRepStats->sysTime,"");                                       
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   memset(text, '_', 132);                                                      
   text1132# = '\0';                                                            
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
                                                                                
   sprintf(text,"%-132.132s","Overflow Statistics");                            
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
   sprintf(text,"%-132.132s","___________________");                            
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Size of largest overflow record",pRepStats->sizeOfLargestOvfRecord,"");   
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Size of largest overflow group",pRepStats->sizeOfLargestOvfGroup,"");     
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Number of overflows on updated database",pRepStats->countOverflows,"");   
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                  
                                                                                
   if (pRepStats->countRejRecsWrit != 0)                                        
   {                                                                            
     sprintf(text,"%-10.10s%-122.122s","","*** WARNING : "                      
       "Updates were REJECTED because the overflow area overflowed!");          
     printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                
                                                                                
     sprintf(text,"%-10.10s%-46.46s%-76d","",                                   
       "The overflow area size is currently (bytes) : ",pProgParms->oflowMax);  
     printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                
   }                                                                            
                                                                                
   sprintf(text,"%-132.132s","Record Counts");                                  
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
   sprintf(text,"%-132.132s","_____________");                                  
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Old master records read",pRepStats->countMasRecsRead,"");                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Update records read",pRepStats->countUpdRecsRead,"");                     
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Update records rejected",pRepStats->countRejRecsWrit,"");                 
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                  
                                                                                
   sprintf(text,"%-10.10s%-50.50s%9d%-63.63s","",                               
     "Updated master records written",pRepStats->countMasRecsWrit,"");          
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'0',text);                  
                                                                                
                                                                                
   sprintf(text,"%-132.132s","Dataset Details");                                
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
   sprintf(text,"%-132.132s","_______________");                                
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,' ',text);                  
                                                                                
   for(i=0; i<NUM_DATASETS; i++)                                                
   {                                                                            
     if (i==0)                                                                  
       asaChar = '0';                                                           
     else                                                                       
       asaChar = ' ';                                                           
                                                                                
     getFileDetails(pRepStats->dataset1i#.ddName, dsname, volSer);              
                                                                                
     sprintf(text,"%-10.10s%-50.50s%-46.46s%-26.26s","",                        
       pRepStats->dataset1i#.description, dsname,volSer);                       
                                                                                
     printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,asaChar,text);            
   }                                                                            
                                                                                
   sprintf(text,"%-132.132s","End of Report "PROG_ID"-01");                     
   printLine(pFilePtrs->pPrintFile,PROG_ID,spoolNum,'-',text);                  
                                                                                
   return;                                                                      
 }                                                                              
                                                                                

