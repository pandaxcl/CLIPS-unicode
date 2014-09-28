   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*              CLIPS Version 6.22  06/15/04           */
   /*                                                     */
   /*                INSTANCE COMMAND MODULE              */
   /*******************************************************/

/*************************************************************/
/* Purpose:  Kernel Interface Commands for Instances         */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Donnell                                     */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */
#include "setup.h"

#if OBJECT_SYSTEM

#include "argacces.h"
#include "classcom.h"
#include "classfun.h"
#include "classinf.h"
#include "envrnmnt.h"
#include "exprnpsr.h"
#include "evaluatn.h"
#include "insfile.h"
#include "insfun.h"
#include "insmngr.h"
#include "insmoddp.h"
#include "insmult.h"
#include "inspsr.h"
#include "lgcldpnd.h"
#include "memalloc.h"
#include "msgcom.h"
#include "msgfun.h"
#include "router.h"
#include "strngrtr.h"
#include "sysdep.h"
#include "utility.h"
#include "commline.h"

#define _INSCOM_SOURCE_
#include "inscom.h"

/* =========================================
   *****************************************
                   CONSTANTS
   =========================================
   ***************************************** */
#define ALL_QUALIFIER      "inherit"

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

#if DEBUGGING_FUNCTIONS
static long ListInstancesInModule(void *,int,char *,char *,BOOLEAN,BOOLEAN);
static long TabulateInstances(void *,int,char *,DEFCLASS *,BOOLEAN,BOOLEAN);
#endif

static void PrintInstance(void *,char *,INSTANCE_TYPE *,char *);
static INSTANCE_SLOT *FindISlotByName(void *,INSTANCE_TYPE *,char *);
static void DeallocateInstanceData(void *);

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/*********************************************************
  NAME         : SetupInstances
  DESCRIPTION  : Initializes instance Hash Table,
                   Function Parsers, and Data Structures
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 *********************************************************/
globle void SetupInstances(
  void *theEnv)
  {
   struct patternEntityRecord instanceInfo = { { "INSTANCE_ADDRESS",
                                                     INSTANCE_ADDRESS,0,0,0,
                                                     PrintInstanceName,
                                                     PrintInstanceLongForm,
                                                     EnvUnmakeInstance,
                                                     NULL,
                                                     EnvGetNextInstance,
                                                     EnvDecrementInstanceCount,
                                                     EnvIncrementInstanceCount,
                                                     NULL,NULL,NULL,NULL
                                                   },
#if INSTANCE_PATTERN_MATCHING
                                                  DecrementObjectBasisCount,
                                                  IncrementObjectBasisCount,
                                                  MatchObjectFunction,
                                                  NetworkSynchronized
#else
                                                  NULL,NULL,NULL,NULL
#endif
                                                };
                                                
   INSTANCE_TYPE dummyInstance = { { NULL }, NULL,NULL, 0, 1 };

   AllocateEnvironmentData(theEnv,INSTANCE_DATA,sizeof(struct instanceData),DeallocateInstanceData);
   
   InstanceData(theEnv)->MkInsMsgPass = TRUE;
   memcpy(&InstanceData(theEnv)->InstanceInfo,&instanceInfo,sizeof(struct patternEntityRecord)); 
   dummyInstance.header.theInfo = &InstanceData(theEnv)->InstanceInfo;    
   memcpy(&InstanceData(theEnv)->DummyInstance,&dummyInstance,sizeof(INSTANCE_TYPE));  

   InitializeInstanceTable(theEnv);
   InstallPrimitive(theEnv,(struct entityRecord *) &InstanceData(theEnv)->InstanceInfo,INSTANCE_ADDRESS);

#if ! RUN_TIME

#if INSTANCE_PATTERN_MATCHING
   EnvDefineFunction2(theEnv,"initialize-instance",'u',
                  PTIEF InactiveInitializeInstance,"InactiveInitializeInstance",NULL);
   EnvDefineFunction2(theEnv,"active-initialize-instance",'u',
                  PTIEF InitializeInstanceCommand,"InitializeInstanceCommand",NULL);
   AddFunctionParser(theEnv,"active-initialize-instance",ParseInitializeInstance);

   EnvDefineFunction2(theEnv,"make-instance",'u',PTIEF InactiveMakeInstance,"InactiveMakeInstance",NULL);
   EnvDefineFunction2(theEnv,"active-make-instance",'u',PTIEF MakeInstanceCommand,"MakeInstanceCommand",NULL);
   AddFunctionParser(theEnv,"active-make-instance",ParseInitializeInstance);

#else
   EnvDefineFunction2(theEnv,"initialize-instance",'u',
                  PTIEF InitializeInstanceCommand,"InitializeInstanceCommand",NULL);
   EnvDefineFunction2(theEnv,"make-instance",'u',PTIEF MakeInstanceCommand,"MakeInstanceCommand",NULL);
#endif
   AddFunctionParser(theEnv,"initialize-instance",ParseInitializeInstance);
   AddFunctionParser(theEnv,"make-instance",ParseInitializeInstance);

   EnvDefineFunction2(theEnv,"init-slots",'u',PTIEF InitSlotsCommand,"InitSlotsCommand","00");

   EnvDefineFunction2(theEnv,"delete-instance",'b',PTIEF DeleteInstanceCommand,
                   "DeleteInstanceCommand","00");
   EnvDefineFunction2(theEnv,"(create-instance)",'b',PTIEF CreateInstanceHandler,
                   "CreateInstanceHandler","00");
   EnvDefineFunction2(theEnv,"unmake-instance",'b',PTIEF UnmakeInstanceCommand,
                   "UnmakeInstanceCommand","1*e");

#if DEBUGGING_FUNCTIONS
   EnvDefineFunction2(theEnv,"instances",'v',PTIEF InstancesCommand,"InstancesCommand","*3w");
   EnvDefineFunction2(theEnv,"ppinstance",'v',PTIEF PPInstanceCommand,"PPInstanceCommand","00");
#endif

   EnvDefineFunction2(theEnv,"symbol-to-instance-name",'u',
                  PTIEF SymbolToInstanceName,"SymbolToInstanceName","11w");
   EnvDefineFunction2(theEnv,"instance-name-to-symbol",'w',
                  PTIEF InstanceNameToSymbol,"InstanceNameToSymbol","11p");
   EnvDefineFunction2(theEnv,"instance-address",'u',PTIEF InstanceAddressCommand,
                   "InstanceAddressCommand","12eep");
   EnvDefineFunction2(theEnv,"instance-addressp",'b',PTIEF InstanceAddressPCommand,
                   "InstanceAddressPCommand","11");
   EnvDefineFunction2(theEnv,"instance-namep",'b',PTIEF InstanceNamePCommand,
                   "InstanceNamePCommand","11");
   EnvDefineFunction2(theEnv,"instance-name",'u',PTIEF InstanceNameCommand,
                   "InstanceNameCommand","11e");
   EnvDefineFunction2(theEnv,"instancep",'b',PTIEF InstancePCommand,"InstancePCommand","11");
   EnvDefineFunction2(theEnv,"instance-existp",'b',PTIEF InstanceExistPCommand,
                   "InstanceExistPCommand","11e");
   EnvDefineFunction2(theEnv,"class",'u',PTIEF ClassCommand,"ClassCommand","11");

   SetupInstanceModDupCommands(theEnv);
   SetupInstanceFileCommands(theEnv);
   SetupInstanceMultifieldCommands(theEnv);

#endif

   AddCleanupFunction(theEnv,"instances",CleanupInstances,0);
   EnvAddResetFunction(theEnv,"instances",DestroyAllInstances,60);
  }
  
/***************************************/
/* DeallocateInstanceData: Deallocates */
/*    environment data for instances.  */
/***************************************/
static void DeallocateInstanceData(
  void *theEnv)
  {
   INSTANCE_TYPE *tmpIPtr, *nextIPtr;
   register unsigned i;
   INSTANCE_SLOT *sp;
   IGARBAGE *tmpGPtr, *nextGPtr;
   struct patternMatch *theMatch, *tmpMatch;
   
   /*=================================*/
   /* Remove the instance hash table. */
   /*=================================*/
   
   rm(theEnv,InstanceData(theEnv)->InstanceTable,
      (int) (sizeof(INSTANCE_TYPE *) * INSTANCE_TABLE_HASH_SIZE));
      
   /*=======================*/
   /* Return all instances. */
   /*=======================*/
   
   tmpIPtr = InstanceData(theEnv)->InstanceList;
   while (tmpIPtr != NULL)
     {
      nextIPtr = tmpIPtr->nxtList;
      
      theMatch = (struct patternMatch *) tmpIPtr->partialMatchList;        
      while (theMatch != NULL)
        {
         tmpMatch = theMatch->next;
         rtn_struct(theEnv,patternMatch,theMatch);
         theMatch = tmpMatch;
        }

#if LOGICAL_DEPENDENCIES
      ReturnEntityDependencies(theEnv,(struct patternEntity *) tmpIPtr);
#endif

      for (i = 0 ; i < tmpIPtr->cls->instanceSlotCount ; i++)
        {
         sp = tmpIPtr->slotAddresses[i];
         if ((sp == &sp->desc->sharedValue) ?
             (--sp->desc->sharedCount == 0) : TRUE)
           {
            if (sp->desc->multiple)
              { ReturnMultifield(theEnv,(MULTIFIELD_PTR) sp->value); }
           }
        }
     
      if (tmpIPtr->cls->instanceSlotCount != 0)
        {
         rm(theEnv,(void *) tmpIPtr->slotAddresses,
            (tmpIPtr->cls->instanceSlotCount * sizeof(INSTANCE_SLOT *)));
         if (tmpIPtr->cls->localInstanceSlotCount != 0)
           {
            rm(theEnv,(void *) tmpIPtr->slots,
               (tmpIPtr->cls->localInstanceSlotCount * sizeof(INSTANCE_SLOT)));
           }
        }
  
      rtn_struct(theEnv,instance,tmpIPtr);

      tmpIPtr = nextIPtr;
     }
     
   /*===============================*/
   /* Get rid of garbage instances. */
   /*===============================*/
   
   tmpGPtr = InstanceData(theEnv)->InstanceGarbageList;
   while (tmpGPtr != NULL)
     {
      nextGPtr = tmpGPtr->nxt;
      rtn_struct(theEnv,instance,tmpGPtr->ins);
      rtn_struct(theEnv,igarbage,tmpGPtr);
      tmpGPtr = nextGPtr;
     }
  }

/*******************************************************************
  NAME         : EnvDeleteInstance
  DESCRIPTION  : DIRECTLY removes a named instance from the
                   hash table and its class's
                   instance list
  INPUTS       : The instance address (NULL to delete all instances)
  RETURNS      : 1 if successful, 0 otherwise
  SIDE EFFECTS : Instance is deallocated
  NOTES        : C interface for deleting instances
 *******************************************************************/
globle BOOLEAN EnvDeleteInstance(
  void *theEnv,
  void *iptr)
  {
   INSTANCE_TYPE *ins,*itmp;
   int success = 1;

   if (iptr != NULL)
     return(QuashInstance(theEnv,(INSTANCE_TYPE *) iptr));
   ins = InstanceData(theEnv)->InstanceList;
   while (ins != NULL)
     {
      itmp = ins;
      ins = ins->nxtList;
      if (QuashInstance(theEnv,(INSTANCE_TYPE *) itmp) == 0)
        success = 0;
     }

   if ((EvaluationData(theEnv)->CurrentEvaluationDepth == 0) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     { PeriodicCleanup(theEnv,TRUE,FALSE); }

   return(success);
  }

/*******************************************************************
  NAME         : EnvUnmakeInstance
  DESCRIPTION  : Removes a named instance via message-passing
  INPUTS       : The instance address (NULL to delete all instances)
  RETURNS      : 1 if successful, 0 otherwise
  SIDE EFFECTS : Instance is deallocated
  NOTES        : C interface for deleting instances
 *******************************************************************/
globle BOOLEAN EnvUnmakeInstance(
  void *theEnv,
  void *iptr)
  {
   INSTANCE_TYPE *ins;
   int success = 1,svmaintain;

   svmaintain = InstanceData(theEnv)->MaintainGarbageInstances;
   InstanceData(theEnv)->MaintainGarbageInstances = TRUE;
   ins = (INSTANCE_TYPE *) iptr;
   if (ins != NULL)
     {
      if (ins->garbage)
        success = 0;
      else
        {
         DirectMessage(theEnv,MessageHandlerData(theEnv)->DELETE_SYMBOL,ins,NULL,NULL);
         if (ins->garbage == 0)
           success = 0;
        }
     }
   else
     {
      ins = InstanceData(theEnv)->InstanceList;
      while (ins != NULL)
        {
         DirectMessage(theEnv,MessageHandlerData(theEnv)->DELETE_SYMBOL,ins,NULL,NULL);
         if (ins->garbage == 0)
           success = 0;
         ins = ins->nxtList;
         while ((ins != NULL) ? ins->garbage : FALSE)
           ins = ins->nxtList;
        }
     }
   InstanceData(theEnv)->MaintainGarbageInstances = svmaintain;
   CleanupInstances(theEnv);

   if ((EvaluationData(theEnv)->CurrentEvaluationDepth == 0) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     { PeriodicCleanup(theEnv,TRUE,FALSE); }

   return(success);
  }

#if DEBUGGING_FUNCTIONS

/*******************************************************************
  NAME         : InstancesCommand
  DESCRIPTION  : Lists all instances associated
                   with a particular class
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instances [<class-name> [inherit]])
 *******************************************************************/
globle void InstancesCommand(
  void *theEnv)
  {
   int argno, inheritFlag = FALSE;
   void *theDefmodule;
   char *className = NULL;
   DATA_OBJECT temp;

   theDefmodule = (void *) EnvGetCurrentModule(theEnv);

   argno = EnvRtnArgCount(theEnv);
   if (argno > 0)
     {
      if (EnvArgTypeCheck(theEnv,"instances",1,SYMBOL,&temp) == FALSE)
        return;
      theDefmodule = EnvFindDefmodule(theEnv,DOToString(temp));
      if ((theDefmodule != NULL) ? FALSE :
          (strcmp(DOToString(temp),"*") != 0))
        {
         SetEvaluationError(theEnv,TRUE);
         ExpectedTypeError1(theEnv,"instances",1,"defmodule name");
         return;
        }
      if (argno > 1)
        {
         if (EnvArgTypeCheck(theEnv,"instances",2,SYMBOL,&temp) == FALSE)
           return;
         className = DOToString(temp);
         if (LookupDefclassAnywhere(theEnv,(struct defmodule *) theDefmodule,className) == NULL)
           {
            if (strcmp(className,"*") == 0)
              className = NULL;
            else
              {
               ClassExistError(theEnv,"instances",className);
                 return;
              }
           }
         if (argno > 2)
           {
            if (EnvArgTypeCheck(theEnv,"instances",3,SYMBOL,&temp) == FALSE)
              return;
            if (strcmp(DOToString(temp),ALL_QUALIFIER) != 0)
              {
               SetEvaluationError(theEnv,TRUE);
               ExpectedTypeError1(theEnv,"instances",3,"keyword \"inherit\"");
               return;
              }
            inheritFlag = TRUE;
           }
        }
     }
   EnvInstances(theEnv,WDISPLAY,theDefmodule,className,inheritFlag);
  }

/********************************************************
  NAME         : PPInstanceCommand
  DESCRIPTION  : Displays the current slot-values
                   of an instance
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (ppinstance <instance>)
 ********************************************************/
globle void PPInstanceCommand(
  void *theEnv)
  {
   INSTANCE_TYPE *ins;

   if (CheckCurrentMessage(theEnv,"ppinstance",TRUE) == FALSE)
     return;
   ins = GetActiveInstance(theEnv);
   if (ins->garbage == 1)
     return;
   PrintInstance(theEnv,WDISPLAY,ins,"\n");
   EnvPrintRouter(theEnv,WDISPLAY,"\n");
  }

/***************************************************************
  NAME         : EnvInstances
  DESCRIPTION  : Lists instances of classes
  INPUTS       : 1) The logical name for the output
                 2) Address of the module (NULL for all classes)
                 3) Name of the class
                    (NULL for all classes in specified module)
                 4) A flag indicating whether to print instances
                    of subclasses or not
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 **************************************************************/
globle void EnvInstances(
  void *theEnv,
  char *logicalName,
  void *theVModule,
  char *className,
  int inheritFlag)
  {
   int id;
   struct defmodule *theModule;
   long count = 0L;

   /* ===========================================
      Grab a traversal id to avoid printing out
      instances twice due to multiple inheritance
      =========================================== */
  if ((id = GetTraversalID(theEnv)) == -1)
    return;
  SaveCurrentModule(theEnv);

   /* ====================================
      For all modules, print out instances
      of specified class(es)
      ==================================== */
   if (theVModule == NULL)
     {
      theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,NULL);
      while (theModule != NULL)
        {
         if (GetHaltExecution(theEnv) == TRUE)
           {
            RestoreCurrentModule(theEnv);
            ReleaseTraversalID(theEnv);
            return;
           }

         EnvPrintRouter(theEnv,logicalName,EnvGetDefmoduleName(theEnv,(void *) theModule));
         EnvPrintRouter(theEnv,logicalName,":\n");
         EnvSetCurrentModule(theEnv,(void *) theModule);
         count += ListInstancesInModule(theEnv,id,logicalName,className,inheritFlag,TRUE);
         theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,(void *) theModule);
        }
     }

   /* ====================================
      For the specified module, print out
      instances of the specified class(es)
      ==================================== */
   else
     {
      EnvSetCurrentModule(theEnv,(void *) theVModule);
      count = ListInstancesInModule(theEnv,id,logicalName,className,inheritFlag,FALSE);
     }

   RestoreCurrentModule(theEnv);
   ReleaseTraversalID(theEnv);
   if (EvaluationData(theEnv)->HaltExecution == FALSE)
     PrintTally(theEnv,logicalName,count,"instance","instances");
  }

#endif

/*********************************************************
  NAME         : EnvMakeInstance
  DESCRIPTION  : C Interface for creating and
                   initializing a class instance
  INPUTS       : The make-instance call string,
                    e.g. "([bill] of man (age 34))"
  RETURNS      : The instance address if instance created,
                    NULL otherwise
  SIDE EFFECTS : Creates the instance and returns
                    the result in caller's buffer
  NOTES        : None
 *********************************************************/
globle void *EnvMakeInstance(
  void *theEnv,
  char *mkstr)
  {
   char *router = "***MKINS***";
   struct token tkn;
   EXPRESSION *top;
   DATA_OBJECT result;

   result.type = SYMBOL;
   result.value = SymbolData(theEnv)->FalseSymbol;
   if (OpenStringSource(theEnv,router,mkstr,0) == 0)
     return(NULL);
   GetToken(theEnv,router,&tkn);
   if (tkn.type == LPAREN)
     {
      top = GenConstant(theEnv,FCALL,(void *) FindFunction(theEnv,"make-instance"));
      if (ParseSimpleInstance(theEnv,top,router) != NULL)
        {
         GetToken(theEnv,router,&tkn);
         if (tkn.type == STOP)
           {
            ExpressionInstall(theEnv,top);
            EvaluateExpression(theEnv,top,&result);
            ExpressionDeinstall(theEnv,top);
           }
         else
           SyntaxErrorMessage(theEnv,"instance definition");
         ReturnExpression(theEnv,top);
        }
     }
   else
     SyntaxErrorMessage(theEnv,"instance definition");
   CloseStringSource(theEnv,router);

   if ((EvaluationData(theEnv)->CurrentEvaluationDepth == 0) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     { PeriodicCleanup(theEnv,TRUE,FALSE); }

   if ((result.type == SYMBOL) && (result.value == SymbolData(theEnv)->FalseSymbol))
     return(NULL);

   return((void *) FindInstanceBySymbol(theEnv,(SYMBOL_HN *) result.value));
  }

/***************************************************************
  NAME         : EnvCreateRawInstance
  DESCRIPTION  : Creates an empty of instance of the specified
                   class.  No slot-overrides or class defaults
                   are applied.
  INPUTS       : 1) Address of class
                 2) Name of the new instance
  RETURNS      : The instance address if instance created,
                    NULL otherwise
  SIDE EFFECTS : Old instance of same name deleted (if possible)
  NOTES        : None
 ***************************************************************/
globle void *EnvCreateRawInstance(
  void *theEnv,
  void *cptr,
  char *iname)
  {
   return((void *) BuildInstance(theEnv,(SYMBOL_HN *) EnvAddSymbol(theEnv,iname),(DEFCLASS *) cptr,FALSE));
  }

/***************************************************************************
  NAME         : EnvFindInstance
  DESCRIPTION  : Looks up a specified instance in the instance hash table
  INPUTS       : Name-string of the instance
  RETURNS      : The address of the found instance, NULL otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************************************/
globle void *EnvFindInstance(
  void *theEnv,
  void *theModule,
  char *iname,
  unsigned searchImports)
  {
   SYMBOL_HN *isym;

   isym = FindSymbolHN(theEnv,iname);
   if (isym == NULL)
     return(NULL);
   if (theModule == NULL)
     theModule = (void *) EnvGetCurrentModule(theEnv);
   return((void *) FindInstanceInModule(theEnv,isym,(struct defmodule *) theModule,
                                        ((struct defmodule *) EnvGetCurrentModule(theEnv)),searchImports));
  }

/***************************************************************************
  NAME         : EnvValidInstanceAddress
  DESCRIPTION  : Determines if an instance address is still valid
  INPUTS       : Instance address
  RETURNS      : 1 if the address is still valid, 0 otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle int EnvValidInstanceAddress(
  void *theEnv,
  void *iptr)
  {
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   return((((INSTANCE_TYPE *) iptr)->garbage == 0) ? 1 : 0);
  }

/***************************************************
  NAME         : EnvDirectGetSlot
  DESCRIPTION  : Gets a slot value
  INPUTS       : 1) Instance address
                 2) Slot name
                 3) Caller's result buffer
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle void EnvDirectGetSlot(
  void *theEnv,
  void *ins,
  char *sname,
  DATA_OBJECT *result)
  {
   INSTANCE_SLOT *sp;

   if (((INSTANCE_TYPE *) ins)->garbage == 1)
     {
      SetEvaluationError(theEnv,TRUE);
      result->type = SYMBOL;
      result->value = SymbolData(theEnv)->FalseSymbol;
      return;
     }
   sp = FindISlotByName(theEnv,(INSTANCE_TYPE *) ins,sname);
   if (sp == NULL)
     {
      SetEvaluationError(theEnv,TRUE);
      result->type = SYMBOL;
      result->value = SymbolData(theEnv)->FalseSymbol;
      return;
     }
   result->type = (unsigned short) sp->type;
   result->value = sp->value;
   if (sp->type == MULTIFIELD)
     {
      result->begin = 0;
      SetpDOEnd(result,GetInstanceSlotLength(sp));
     }
   PropagateReturnValue(theEnv,result);
  }

/*********************************************************
  NAME         : EnvDirectPutSlot
  DESCRIPTION  : Gets a slot value
  INPUTS       : 1) Instance address
                 2) Slot name
                 3) Caller's new value buffer
  RETURNS      : TRUE if put successful, FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 *********************************************************/
globle int EnvDirectPutSlot(
  void *theEnv,
  void *ins,
  char *sname,
  DATA_OBJECT *val)
  {
   INSTANCE_SLOT *sp;
   DATA_OBJECT junk;

   if ((((INSTANCE_TYPE *) ins)->garbage == 1) || (val == NULL))
     {
      SetEvaluationError(theEnv,TRUE);
      return(FALSE);
     }
   sp = FindISlotByName(theEnv,(INSTANCE_TYPE *) ins,sname);
   if (sp == NULL)
     {
      SetEvaluationError(theEnv,TRUE);
      return(FALSE);
     }

   if (PutSlotValue(theEnv,(INSTANCE_TYPE *) ins,sp,val,&junk,"external put"))
     {
      if ((EvaluationData(theEnv)->CurrentEvaluationDepth == 0) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
          (EvaluationData(theEnv)->CurrentExpression == NULL))
        { PeriodicCleanup(theEnv,TRUE,FALSE); }
      return(TRUE);
     }
   return(FALSE);
  }

/***************************************************
  NAME         : GetInstanceName
  DESCRIPTION  : Returns name of instance
  INPUTS       : Pointer to instance
  RETURNS      : Name of instance
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle char *EnvGetInstanceName(
  void *theEnv,
  void *iptr)
  {
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     return(NULL);
   return(ValueToString(((INSTANCE_TYPE *) iptr)->name));
  }

/***************************************************
  NAME         : EnvGetInstanceClass
  DESCRIPTION  : Returns class of instance
  INPUTS       : Pointer to instance
  RETURNS      : Pointer to class of instance
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle void *EnvGetInstanceClass(
  void *theEnv,
  void *iptr)
  {
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     return(NULL);
   return((void *) ((INSTANCE_TYPE *) iptr)->cls);
  }

/***************************************************
  NAME         : GetGlobalNumberOfInstances
  DESCRIPTION  : Returns the total number of
                   instances in all modules
  INPUTS       : None
  RETURNS      : The instance count
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle unsigned long GetGlobalNumberOfInstances(
  void *theEnv)
  {
   return(InstanceData(theEnv)->GlobalNumberOfInstances);
  }

/***************************************************
  NAME         : EnvGetNextInstance
  DESCRIPTION  : Returns next instance in list
                 (or first instance in list)
  INPUTS       : Pointer to previous instance
                 (or NULL to get first instance)
  RETURNS      : The next instance or first instance
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle void *EnvGetNextInstance(
  void *theEnv,
  void *iptr)
  {
   if (iptr == NULL)
     return((void *) InstanceData(theEnv)->InstanceList);
   if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     return(NULL);
   return((void *) ((INSTANCE_TYPE *) iptr)->nxtList);
  }

/***************************************************
  NAME         : GetNextInstanceInScope
  DESCRIPTION  : Returns next instance in list
                 (or first instance in list)
                 which class is in scope
  INPUTS       : Pointer to previous instance
                 (or NULL to get first instance)
  RETURNS      : The next instance or first instance
                 which class is in scope of the
                 current module
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle void *GetNextInstanceInScope(
  void *theEnv,
  void *iptr)
  {
   INSTANCE_TYPE *ins = (INSTANCE_TYPE *) iptr;

   if (ins == NULL)
     ins = InstanceData(theEnv)->InstanceList;
   else if (ins->garbage)
     return(NULL);
   else
     ins = ins->nxtList;
   while (ins != NULL)
     {
      if (DefclassInScope(theEnv,ins->cls,NULL))
        return((void *) ins);
      ins = ins->nxtList;
     }
   return(NULL);
  }

/***************************************************
  NAME         : EnvGetNextInstanceInClass
  DESCRIPTION  : Finds next instance of class
                 (or first instance of class)
  INPUTS       : 1) Class address
                 2) Instance address
                    (NULL to get first instance)
  RETURNS      : The next or first class instance
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle void *EnvGetNextInstanceInClass(
  void *theEnv,
  void *cptr,
  void *iptr)
  {
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   if (iptr == NULL)
     return((void *) ((DEFCLASS *) cptr)->instanceList);
   if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     return(NULL);
   return((void *) ((INSTANCE_TYPE *) iptr)->nxtClass);
  }

/***************************************************
  NAME         : EnvGetNextInstanceInClassAndSubclasses
  DESCRIPTION  : Finds next instance of class
                 (or first instance of class) and
                 all of its subclasses
  INPUTS       : 1) Class address
                 2) Instance address
                    (NULL to get first instance)
  RETURNS      : The next or first class instance
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle void *EnvGetNextInstanceInClassAndSubclasses(
  void *theEnv,
  void **cptr,
  void *iptr,
  DATA_OBJECT *iterationInfo)
  {
   INSTANCE_TYPE *nextInstance;
   DEFCLASS *theClass;
   
   theClass = (DEFCLASS *) *cptr;
   
   if (iptr == NULL)
     {
      ClassSubclassAddresses(theEnv,theClass,iterationInfo,TRUE);
      nextInstance = theClass->instanceList;
     }
   else if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     { nextInstance = NULL; }
   else
     { nextInstance = ((INSTANCE_TYPE *) iptr)->nxtClass; }
     
   while ((nextInstance == NULL) && 
          (GetpDOBegin(iterationInfo) <= GetpDOEnd(iterationInfo)))
     {
      theClass = (struct defclass *) GetMFValue(DOPToPointer(iterationInfo),
                                                GetpDOBegin(iterationInfo));
      *cptr = theClass;
      SetpDOBegin(iterationInfo,GetpDOBegin(iterationInfo) + 1);
      nextInstance = theClass->instanceList;
     }
          
   return(nextInstance);
  }
  
/***************************************************
  NAME         : EnvGetInstancePPForm
  DESCRIPTION  : Writes slot names and values to
                  caller's buffer
  INPUTS       : 1) Caller's buffer
                 2) Size of buffer (not including
                    space for terminating '\0')
                 3) Instance address
  RETURNS      : Nothing useful
  SIDE EFFECTS : Caller's buffer written
  NOTES        : None
 ***************************************************/
globle void EnvGetInstancePPForm(
  void *theEnv,
  char *buf,
  unsigned buflen,
  void *iptr)
  {
   char *pbuf = "***InstancePPForm***";

   if (((INSTANCE_TYPE *) iptr)->garbage == 1)
     return;
   if (OpenStringDestination(theEnv,pbuf,buf,buflen+1) == 0)
     return;
   PrintInstance(theEnv,pbuf,(INSTANCE_TYPE *) iptr," ");
   CloseStringDestination(theEnv,pbuf);
  }

/*********************************************************
  NAME         : ClassCommand
  DESCRIPTION  : Returns the class of an instance
  INPUTS       : Caller's result buffer
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (class <object>)
                 Can also be called by (type <object>)
                   if you have generic functions installed
 *********************************************************/
globle void ClassCommand(
  void *theEnv,
  DATA_OBJECT *result)
  {
   INSTANCE_TYPE *ins;
   char *func;
   DATA_OBJECT temp;

   func = ValueToString(((struct FunctionDefinition *)
                       EvaluationData(theEnv)->CurrentExpression->value)->callFunctionName);
   result->type = SYMBOL;
   result->value = SymbolData(theEnv)->FalseSymbol;
   EvaluateExpression(theEnv,GetFirstArgument(),&temp);
   if (temp.type == INSTANCE_ADDRESS)
     {
      ins = (INSTANCE_TYPE *) temp.value;
      if (ins->garbage == 1)
        {
         StaleInstanceAddress(theEnv,func,0);
         SetEvaluationError(theEnv,TRUE);
         return;
        }
      result->value = (void *) GetDefclassNamePointer((void *) ins->cls);
     }
   else if (temp.type == INSTANCE_NAME)
     {
      ins = FindInstanceBySymbol(theEnv,(SYMBOL_HN *) temp.value);
      if (ins == NULL)
        {
         NoInstanceError(theEnv,ValueToString(temp.value),func);
         return;
        }
      result->value = (void *) GetDefclassNamePointer((void *) ins->cls);
     }
   else
     {
      switch (temp.type)
        {
         case INTEGER          :
         case FLOAT            :
         case SYMBOL           :
         case STRING           :
         case MULTIFIELD       :
         case EXTERNAL_ADDRESS :
         case FACT_ADDRESS     :
                          result->value = (void *)
                                           GetDefclassNamePointer((void *)
                                            DefclassData(theEnv)->PrimitiveClassMap[temp.type]);
                         return;
         default       : PrintErrorID(theEnv,"INSCOM",1,FALSE);
                         EnvPrintRouter(theEnv,WERROR,"Undefined type in function ");
                         EnvPrintRouter(theEnv,WERROR,func);
                         EnvPrintRouter(theEnv,WERROR,".\n");
                         SetEvaluationError(theEnv,TRUE);
        }
     }
  }
  
/******************************************************
  NAME         : CreateInstanceHandler
  DESCRIPTION  : Message handler called after instance creation
  INPUTS       : None
  RETURNS      : TRUE if successful,
                 FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : Does nothing. Provided so it can be overridden.
 ******************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle BOOLEAN CreateInstanceHandler(
  void *theEnv)
  {
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   return(TRUE);
  }

/******************************************************
  NAME         : DeleteInstanceCommand
  DESCRIPTION  : Removes a named instance from the
                   hash table and its class's
                   instance list
  INPUTS       : None
  RETURNS      : TRUE if successful,
                 FALSE otherwise
  SIDE EFFECTS : Instance is deallocated
  NOTES        : This is an internal function that
                   only be called by a handler
 ******************************************************/
globle BOOLEAN DeleteInstanceCommand(
  void *theEnv)
  {
   if (CheckCurrentMessage(theEnv,"delete-instance",TRUE))
     return(QuashInstance(theEnv,GetActiveInstance(theEnv)));
   return(FALSE);
  }

/********************************************************************
  NAME         : UnmakeInstanceCommand
  DESCRIPTION  : Uses message-passing to delete the
                   specified instance
  INPUTS       : None
  RETURNS      : TRUE if successful, FALSE otherwise
  SIDE EFFECTS : Instance is deallocated
  NOTES        : Syntax: (unmake-instance <instance-expression>+ | *)
 ********************************************************************/
globle BOOLEAN UnmakeInstanceCommand(
  void *theEnv)
  {
   EXPRESSION *theArgument;
   DATA_OBJECT theResult;
   INSTANCE_TYPE *ins;
   int argNumber = 1,rtn = TRUE;

   theArgument = GetFirstArgument();
   while (theArgument != NULL)
     {
      EvaluateExpression(theEnv,theArgument,&theResult);
      if ((theResult.type == INSTANCE_NAME) || (theResult.type == SYMBOL))
        {
         ins = FindInstanceBySymbol(theEnv,(SYMBOL_HN *) theResult.value);
         if ((ins == NULL) ? (strcmp(DOToString(theResult),"*") != 0) : FALSE)
           {
            NoInstanceError(theEnv,DOToString(theResult),"unmake-instance");
            return(FALSE);
           }
         }
      else if (theResult.type == INSTANCE_ADDRESS)
        {
         ins = (INSTANCE_TYPE *) theResult.value;
         if (ins->garbage)
           {
            StaleInstanceAddress(theEnv,"unmake-instance",0);
            SetEvaluationError(theEnv,TRUE);
            return(FALSE);
           }
        }
      else
        {
         ExpectedTypeError1(theEnv,"retract",argNumber,"instance-address, instance-name, or the symbol *");
         SetEvaluationError(theEnv,TRUE);
         return(FALSE);
        }
      if (EnvUnmakeInstance(theEnv,ins) == FALSE)
        rtn = FALSE;
      if (ins == NULL)
        return(rtn);
      argNumber++;
      theArgument = GetNextArgument(theArgument);
     }
   return(rtn);
  }

/*****************************************************************
  NAME         : SymbolToInstanceName
  DESCRIPTION  : Converts a symbol from type SYMBOL
                   to type INSTANCE_NAME
  INPUTS       : The address of the value buffer
  RETURNS      : The new INSTANCE_NAME symbol
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (symbol-to-instance-name <symbol>)
 *****************************************************************/
globle void SymbolToInstanceName(
  void *theEnv,
  DATA_OBJECT *result)
  {
   if (EnvArgTypeCheck(theEnv,"symbol-to-instance-name",1,SYMBOL,result) == FALSE)
     {
      SetpType(result,SYMBOL);
      SetpValue(result,SymbolData(theEnv)->FalseSymbol);
      return;
     }
   SetpType(result,INSTANCE_NAME);
  }

/*****************************************************************
  NAME         : InstanceNameToSymbol
  DESCRIPTION  : Converts a symbol from type INSTANCE_NAME
                   to type SYMBOL
  INPUTS       : None
  RETURNS      : Symbol FALSE on errors - or converted instance name
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instance-name-to-symbol <iname>)
 *****************************************************************/
globle SYMBOL_HN *InstanceNameToSymbol(
  void *theEnv)
  {
   DATA_OBJECT result;

   if (EnvArgTypeCheck(theEnv,"instance-name-to-symbol",1,INSTANCE_NAME,&result) == FALSE)
     return((SYMBOL_HN *) SymbolData(theEnv)->FalseSymbol);
   return((SYMBOL_HN *) result.value);
  }

/*********************************************************************************
  NAME         : InstanceAddressCommand
  DESCRIPTION  : Returns the address of an instance
  INPUTS       : The address of the value buffer
  RETURNS      : Nothing useful
  SIDE EFFECTS : Stores instance address in caller's buffer
  NOTES        : H/L Syntax : (instance-address [<module-name>] <instance-name>)
 *********************************************************************************/
globle void InstanceAddressCommand(
  void *theEnv,
  DATA_OBJECT *result)
  {
   INSTANCE_TYPE *ins;
   DATA_OBJECT temp;
   struct defmodule *theModule;
   unsigned searchImports;

   result->type = SYMBOL;
   result->value = SymbolData(theEnv)->FalseSymbol;
   if (EnvRtnArgCount(theEnv) > 1)
     {
      if (EnvArgTypeCheck(theEnv,"instance-address",1,SYMBOL,&temp) == FALSE)
        return;
      theModule = (struct defmodule *) EnvFindDefmodule(theEnv,DOToString(temp));
      if ((theModule == NULL) ? (strcmp(DOToString(temp),"*") != 0) : FALSE)
        {
         ExpectedTypeError1(theEnv,"instance-address",1,"module name");
         SetEvaluationError(theEnv,TRUE);
         return;
        }
      if (theModule == NULL)
        {
         searchImports = TRUE;
         theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
        }
      else
        searchImports = FALSE;
      if (EnvArgTypeCheck(theEnv,"instance-address",2,INSTANCE_NAME,&temp)
             == FALSE)
        return;
      ins = FindInstanceInModule(theEnv,(SYMBOL_HN *) temp.value,theModule,
                                 ((struct defmodule *) EnvGetCurrentModule(theEnv)),searchImports);
      if (ins != NULL)
        {
         result->type = INSTANCE_ADDRESS;
         result->value = (void *) ins;
        }
      else
        NoInstanceError(theEnv,ValueToString(temp.value),"instance-address");
     }
   else if (EnvArgTypeCheck(theEnv,"instance-address",1,INSTANCE_OR_INSTANCE_NAME,&temp))
     {
      if (temp.type == INSTANCE_ADDRESS)
        {
         ins = (INSTANCE_TYPE *) temp.value;
         if (ins->garbage == 0)
           {
            result->type = INSTANCE_ADDRESS;
            result->value = temp.value;
           }
         else
           {
            StaleInstanceAddress(theEnv,"instance-address",0);
            SetEvaluationError(theEnv,TRUE);
           }
        }
      else
        {
         ins = FindInstanceBySymbol(theEnv,(SYMBOL_HN *) temp.value);
         if (ins != NULL)
           {
            result->type = INSTANCE_ADDRESS;
            result->value = (void *) ins;
           }
         else
           NoInstanceError(theEnv,ValueToString(temp.value),"instance-address");
        }
     }
  }

/***************************************************************
  NAME         : InstanceNameCommand
  DESCRIPTION  : Gets the name of an INSTANCE
  INPUTS       : The address of the value buffer
  RETURNS      : The INSTANCE_NAME symbol
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instance-name <instance>)
 ***************************************************************/
globle void InstanceNameCommand(
  void *theEnv,
  DATA_OBJECT *result)
  {
   INSTANCE_TYPE *ins;
   DATA_OBJECT temp;

   result->type = SYMBOL;
   result->value = SymbolData(theEnv)->FalseSymbol;
   if (EnvArgTypeCheck(theEnv,"instance-name",1,INSTANCE_OR_INSTANCE_NAME,&temp) == FALSE)
     return;
   if (temp.type == INSTANCE_ADDRESS)
     {
      ins = (INSTANCE_TYPE *) temp.value;
      if (ins->garbage == 1)
        {
         StaleInstanceAddress(theEnv,"instance-name",0);
         SetEvaluationError(theEnv,TRUE);
         return;
        }
     }
   else
     {
      ins = FindInstanceBySymbol(theEnv,(SYMBOL_HN *) temp.value);
      if (ins == NULL)
        {
         NoInstanceError(theEnv,ValueToString(temp.value),"instance-name");
         return;
        }
     }
   result->type = INSTANCE_NAME;
   result->value = (void *) ins->name;
  }

/**************************************************************
  NAME         : InstanceAddressPCommand
  DESCRIPTION  : Determines if a value is of type INSTANCE
  INPUTS       : None
  RETURNS      : TRUE if type INSTANCE_ADDRESS, FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instance-addressp <arg>)
 **************************************************************/
globle BOOLEAN InstanceAddressPCommand(
  void *theEnv)
  {
   DATA_OBJECT temp;

   EvaluateExpression(theEnv,GetFirstArgument(),&temp);
   return((GetType(temp) == INSTANCE_ADDRESS) ? TRUE : FALSE);
  }

/**************************************************************
  NAME         : InstanceNamePCommand
  DESCRIPTION  : Determines if a value is of type INSTANCE_NAME
  INPUTS       : None
  RETURNS      : TRUE if type INSTANCE_NAME, FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instance-namep <arg>)
 **************************************************************/
globle BOOLEAN InstanceNamePCommand(
  void *theEnv)
  {
   DATA_OBJECT temp;

   EvaluateExpression(theEnv,GetFirstArgument(),&temp);
   return((GetType(temp) == INSTANCE_NAME) ? TRUE : FALSE);
  }

/*****************************************************************
  NAME         : InstancePCommand
  DESCRIPTION  : Determines if a value is of type INSTANCE_ADDRESS
                   or INSTANCE_NAME
  INPUTS       : None
  RETURNS      : TRUE if type INSTANCE_NAME or INSTANCE_ADDRESS,
                     FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instancep <arg>)
 *****************************************************************/
globle BOOLEAN InstancePCommand(
  void *theEnv)
  {
   DATA_OBJECT temp;

   EvaluateExpression(theEnv,GetFirstArgument(),&temp);
   if ((GetType(temp) == INSTANCE_NAME) || (GetType(temp) == INSTANCE_ADDRESS))
     return(TRUE);
   return(FALSE);
  }

/********************************************************
  NAME         : InstanceExistPCommand
  DESCRIPTION  : Determines if an instance exists
  INPUTS       : None
  RETURNS      : TRUE if instance exists, FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : H/L Syntax : (instance-existp <arg>)
 ********************************************************/
globle BOOLEAN InstanceExistPCommand(
  void *theEnv)
  {
   DATA_OBJECT temp;

   EvaluateExpression(theEnv,GetFirstArgument(),&temp);
   if (temp.type == INSTANCE_ADDRESS)
     return((((INSTANCE_TYPE *) temp.value)->garbage == 0) ? TRUE : FALSE);
   if ((temp.type == INSTANCE_NAME) || (temp.type == SYMBOL))
     return((FindInstanceBySymbol(theEnv,(SYMBOL_HN *) temp.value) != NULL) ?
             TRUE : FALSE);
   ExpectedTypeError1(theEnv,"instance-existp",1,"instance name, instance address or symbol");
   SetEvaluationError(theEnv,TRUE);
   return(FALSE);
  }

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

#if DEBUGGING_FUNCTIONS

/***************************************************
  NAME         : ListInstancesInModule
  DESCRIPTION  : List instances of specified
                 class(es) in a module
  INPUTS       : 1) Traversal id to avoid multiple
                    passes over same class
                 2) Logical name of output
                 3) The name of the class
                    (NULL for all classes)
                 4) Flag indicating whether to
                    include instances of subclasses
                 5) A flag indicating whether to
                    indent because of module name
  RETURNS      : The number of instances listed
  SIDE EFFECTS : Instances listed to logical output
  NOTES        : Assumes defclass scope flags
                 are up to date
 ***************************************************/
static long ListInstancesInModule(
  void *theEnv,
  int id,
  char *logicalName,
  char *className,
  BOOLEAN inheritFlag,
  BOOLEAN allModulesFlag)
  {
   void *theDefclass,*theInstance;
   long count = 0L;

   /* ===================================
      For the specified module, print out
      instances of all the classes
      =================================== */
   if (className == NULL)
     {
      /* ==============================================
         If instances are being listed for all modules,
         only list the instances of classes in this
         module (to avoid listing instances twice)
         ============================================== */
      if (allModulesFlag)
        {
         for (theDefclass = EnvGetNextDefclass(theEnv,NULL) ;
              theDefclass != NULL ;
              theDefclass = EnvGetNextDefclass(theEnv,theDefclass))
           count += TabulateInstances(theEnv,id,logicalName,
                        (DEFCLASS *) theDefclass,FALSE,allModulesFlag);
        }

      /* ===================================================
         If instances are only be listed for one module,
         list all instances visible to the module (including
         ones belonging to classes in other modules)
         =================================================== */
      else
        {
         theInstance = GetNextInstanceInScope(theEnv,NULL);
         while (theInstance != NULL)
           {
            if (GetHaltExecution(theEnv) == TRUE)
              { return(count); }

            count++;
            PrintInstanceNameAndClass(theEnv,logicalName,(INSTANCE_TYPE *) theInstance,TRUE);
            theInstance = GetNextInstanceInScope(theEnv,theInstance);
           }
        }
     }

   /* ===================================
      For the specified module, print out
      instances of the specified class
      =================================== */
   else
     {
      theDefclass = (void *) LookupDefclassAnywhere(theEnv,((struct defmodule *) EnvGetCurrentModule(theEnv)),className);
      if (theDefclass != NULL)
        {
         count += TabulateInstances(theEnv,id,logicalName,
                      (DEFCLASS *) theDefclass,inheritFlag,allModulesFlag);
        }
      else if (! allModulesFlag)
        ClassExistError(theEnv,"instances",className);
     }
   return(count);
  }

/******************************************************
  NAME         : TabulateInstances
  DESCRIPTION  : Displays all instances for a class
  INPUTS       : 1) The traversal id for the classes
                 2) The logical name of the output
                 3) The class address
                 4) A flag indicating whether to
                    print out instances of subclasses
                    or not.
                 5) A flag indicating whether to
                    indent because of module name
  RETURNS      : The number of instances (including
                    subclasses' instances)
  SIDE EFFECTS : None
  NOTES        : None
 ******************************************************/
static long TabulateInstances(
  void *theEnv,
  int id,
  char *logicalName,
  DEFCLASS *cls,
  BOOLEAN inheritFlag,
  BOOLEAN allModulesFlag)
  {
   INSTANCE_TYPE *ins;
   register unsigned i;
   long count = 0;

   if (TestTraversalID(cls->traversalRecord,id))
     return(0L);
   SetTraversalID(cls->traversalRecord,id);
   for (ins = cls->instanceList ; ins != NULL ; ins = ins->nxtClass)
     {
      if (EvaluationData(theEnv)->HaltExecution)
        return(count);
      if (allModulesFlag)
        EnvPrintRouter(theEnv,logicalName,"   ");
      PrintInstanceNameAndClass(theEnv,logicalName,ins,TRUE);
      count++;
     }
   if (inheritFlag)
     {
      for (i = 0 ; i < cls->directSubclasses.classCount ; i++)
        {
         if (EvaluationData(theEnv)->HaltExecution)
           return(count);
         count += TabulateInstances(theEnv,id,logicalName,
                     cls->directSubclasses.classArray[i],inheritFlag,allModulesFlag);
        }
     }
   return(count);
  }

#endif

/***************************************************
  NAME         : PrintInstance
  DESCRIPTION  : Displays an instance's slots
  INPUTS       : 1) Logical name for output
                 2) Instance address
                 3) String used to separate
                    slot printouts
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : Assumes instance is valid
 ***************************************************/
static void PrintInstance(
  void *theEnv,
  char *logicalName,
  INSTANCE_TYPE *ins,
  char *separator)
  {
   register unsigned i;
   register INSTANCE_SLOT *sp;

   PrintInstanceNameAndClass(theEnv,logicalName,ins,FALSE);
   for (i = 0 ; i < ins->cls->instanceSlotCount ; i++)
     {
      EnvPrintRouter(theEnv,logicalName,separator);
      sp = ins->slotAddresses[i];
      EnvPrintRouter(theEnv,logicalName,"(");
      EnvPrintRouter(theEnv,logicalName,ValueToString(sp->desc->slotName->name));
      if (sp->type != MULTIFIELD)
        {
         EnvPrintRouter(theEnv,logicalName," ");
         PrintAtom(theEnv,logicalName,(int) sp->type,sp->value);
        }
      else if (GetInstanceSlotLength(sp) != 0)
        {
         EnvPrintRouter(theEnv,logicalName," ");
         PrintMultifield(theEnv,logicalName,(MULTIFIELD_PTR) sp->value,0,
                         (long) (GetInstanceSlotLength(sp) - 1),FALSE);
        }
      EnvPrintRouter(theEnv,logicalName,")");
     }
  }

/***************************************************
  NAME         : FindISlotByName
  DESCRIPTION  : Looks up an instance slot by
                   instance name and slot name
  INPUTS       : 1) Instance address
                 2) Instance name-string
  RETURNS      : The instance slot address, NULL if
                   does not exist
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
static INSTANCE_SLOT *FindISlotByName(
  void *theEnv,
  INSTANCE_TYPE *ins,
  char *sname)
  {
   SYMBOL_HN *ssym;

   ssym = FindSymbolHN(theEnv,sname);
   if (ssym == NULL)
     return(NULL);
   return(FindInstanceSlot(theEnv,ins,ssym));
  }

#endif

