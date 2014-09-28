   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.22  06/15/04            */
   /*                                                     */
   /*                  DEFMODULE MODULE                   */
   /*******************************************************/

/*************************************************************/
/* Purpose: Defines basic defmodule primitive functions such */
/*   as allocating and deallocating, traversing, and finding */
/*   defmodule data structures.                              */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Donnell                                     */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#define _MODULDEF_SOURCE_

#include "setup.h"

#include <stdio.h>
#include <string.h>
#define _STDIO_INCLUDED_

#include "memalloc.h"
#include "constant.h"
#include "router.h"
#include "extnfunc.h"
#include "argacces.h"
#include "constrct.h"
#include "modulpsr.h"
#include "modulcmp.h"
#include "modulbsc.h"
#include "utility.h"
#include "envrnmnt.h"

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "bload.h"
#include "modulbin.h"
#endif

#include "moduldef.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if (! RUN_TIME)
   static void                       ReturnDefmodule(void *,struct defmodule *,BOOLEAN);
#endif
   static void                       DeallocateDefmoduleData(void *);

/**************************************************************/
/* InitializeDefmodules: Initializes the defmodule construct. */
/**************************************************************/
globle void AllocateDefmoduleGlobals(
  void *theEnv)
  {
   AllocateEnvironmentData(theEnv,DEFMODULE_DATA,sizeof(struct defmoduleData),NULL);
   AddEnvironmentCleanupFunction(theEnv,"defmodules",DeallocateDefmoduleData,-1000);
   DefmoduleData(theEnv)->CallModuleChangeFunctions = TRUE;
   DefmoduleData(theEnv)->MainModuleRedefinable = TRUE;
  }

/****************************************************/
/* DeallocateDefmoduleData: Deallocates environment */
/*    data for the defmodule construct.             */
/****************************************************/
static void DeallocateDefmoduleData(
  void *theEnv)
  {
   struct moduleStackItem *tmpMSPtr, *nextMSPtr;
   struct moduleItem *tmpMIPtr, *nextMIPtr;
#if (! RUN_TIME)
   struct defmodule *tmpDMPtr, *nextDMPtr;
   struct portConstructItem *tmpPCPtr, *nextPCPtr;
#endif
#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   int i;
   unsigned long space;
#endif

#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   for (i = 0; i < DefmoduleData(theEnv)->BNumberOfDefmodules; i++)
     {
      if (DefmoduleData(theEnv)->DefmoduleArray[i].itemsArray != NULL)
        { 
         rm(theEnv,DefmoduleData(theEnv)->DefmoduleArray[i].itemsArray,
            sizeof(void *) * GetNumberOfModuleItems(theEnv));
        }
     }

   space = DefmoduleData(theEnv)->BNumberOfDefmodules * sizeof(struct defmodule);
   if (space != 0) 
     {
      genlongfree(theEnv,(void *) DefmoduleData(theEnv)->DefmoduleArray,space);
      DefmoduleData(theEnv)->ListOfDefmodules = NULL;
     }

   space = DefmoduleData(theEnv)->NumberOfPortItems * sizeof(struct portItem);
   if (space != 0) genlongfree(theEnv,(void *) DefmoduleData(theEnv)->PortItemArray,space);
#endif

#if (! RUN_TIME) && (! BLOAD_ONLY)
   tmpDMPtr = DefmoduleData(theEnv)->ListOfDefmodules;
   while (tmpDMPtr != NULL)
     {
      nextDMPtr = tmpDMPtr->next;
      ReturnDefmodule(theEnv,tmpDMPtr,TRUE);
      tmpDMPtr = nextDMPtr;
     }

   tmpPCPtr = DefmoduleData(theEnv)->ListOfPortConstructItems;
   while (tmpPCPtr != NULL)
     {
      nextPCPtr = tmpPCPtr->next;
      rtn_struct(theEnv,portConstructItem,tmpPCPtr);
      tmpPCPtr = nextPCPtr;
     }
#endif

   tmpMSPtr = DefmoduleData(theEnv)->ModuleStack;
   while (tmpMSPtr != NULL)
     {
      nextMSPtr = tmpMSPtr->next;
      rtn_struct(theEnv,moduleStackItem,tmpMSPtr);
      tmpMSPtr = nextMSPtr;
     }

   tmpMIPtr = DefmoduleData(theEnv)->ListOfModuleItems;
   while (tmpMIPtr != NULL)
     {
      nextMIPtr = tmpMIPtr->next;
      rtn_struct(theEnv,moduleItem,tmpMIPtr);
      tmpMIPtr = nextMIPtr;
     }
     
#if (! RUN_TIME) && (! BLOAD_ONLY)
   DeallocateCallList(theEnv,DefmoduleData(theEnv)->AfterModuleDefinedFunctions);
#endif
   DeallocateCallList(theEnv,DefmoduleData(theEnv)->AfterModuleChangeFunctions);
  }
  
/**************************************************************/
/* InitializeDefmodules: Initializes the defmodule construct. */
/**************************************************************/
globle void InitializeDefmodules(
  void *theEnv)
  {
   DefmoduleBasicCommands(theEnv);

#if (! RUN_TIME)
   CreateMainModule(theEnv);
#endif

#if DEFMODULE_CONSTRUCT && (! RUN_TIME) && (! BLOAD_ONLY)
   AddConstruct(theEnv,"defmodule","defmodules",ParseDefmodule,NULL,NULL,NULL,NULL,
                                                        NULL,NULL,NULL,NULL,NULL);
#endif

#if (! RUN_TIME) && DEFMODULE_CONSTRUCT
   EnvDefineFunction2(theEnv,"get-current-module", 'w',
                   PTIEF GetCurrentModuleCommand,
                   "GetCurrentModuleCommand", "00");

   EnvDefineFunction2(theEnv,"set-current-module", 'w',
                   PTIEF SetCurrentModuleCommand,
                   "SetCurrentModuleCommand", "11w");
#endif
  }

/******************************************************/
/* RegisterModuleItem: Called to register a construct */
/*   which can be placed within a module.             */
/******************************************************/
globle int RegisterModuleItem(
   void *theEnv,
   char *theItem,
   void *(*allocateFunction)(void *),
   void (*freeFunction)(void *,void *),
   void *(*bloadModuleReference)(void *,int),
   void  (*constructsToCModuleReference)(void *,FILE *,int,int,int),
   void *(*findFunction)(void *,char *))
  {
   struct moduleItem *newModuleItem;

   newModuleItem = get_struct(theEnv,moduleItem);
   newModuleItem->name = theItem;
   newModuleItem->allocateFunction = allocateFunction;
   newModuleItem->freeFunction = freeFunction;
   newModuleItem->bloadModuleReference = bloadModuleReference;
   newModuleItem->constructsToCModuleReference = constructsToCModuleReference;
   newModuleItem->findFunction = findFunction;
   newModuleItem->moduleIndex = DefmoduleData(theEnv)->NumberOfModuleItems++;
   newModuleItem->next = NULL;

   if (DefmoduleData(theEnv)->LastModuleItem == NULL)
     {
      DefmoduleData(theEnv)->ListOfModuleItems = newModuleItem;
      DefmoduleData(theEnv)->LastModuleItem = newModuleItem;
     }
   else
     {
      DefmoduleData(theEnv)->LastModuleItem->next = newModuleItem;
      DefmoduleData(theEnv)->LastModuleItem = newModuleItem;
     }

   return(newModuleItem->moduleIndex);
  }

/***********************************************************/
/* GetListOfModuleItems: Returns the list of module items. */
/***********************************************************/
globle struct moduleItem *GetListOfModuleItems(
  void *theEnv)
  {
   return (DefmoduleData(theEnv)->ListOfModuleItems);
  }

/***************************************************************/
/* GetNumberOfModuleItems: Returns the number of module items. */
/***************************************************************/
globle int GetNumberOfModuleItems(
  void *theEnv)
  {
   return (DefmoduleData(theEnv)->NumberOfModuleItems);
  }

/********************************************************/
/* FindModuleItem: Finds the module item data structure */
/*   corresponding to the specified name.               */
/********************************************************/
globle struct moduleItem *FindModuleItem(
  void *theEnv,
  char *theName)
  {
   struct moduleItem *theModuleItem;

   for (theModuleItem = DefmoduleData(theEnv)->ListOfModuleItems;
        theModuleItem != NULL;
        theModuleItem = theModuleItem->next)
     { if (strcmp(theModuleItem->name,theName) == 0) return(theModuleItem); }

   return(NULL);
  }

/******************************************/
/* EnvGetCurrentModule: Returns a pointer */
/*   to the current module.               */
/******************************************/
globle void *EnvGetCurrentModule(
  void *theEnv)
  {
   return ((void *) DefmoduleData(theEnv)->CurrentModule);
  }

/**************************************************************/
/* EnvSetCurrentModule: Sets the value of the current module. */
/**************************************************************/
globle void *EnvSetCurrentModule(
  void *theEnv,
  void *xNewValue)
  {
   struct defmodule *newValue = (struct defmodule *) xNewValue;
   struct callFunctionItem *changeFunctions;
   void *rv;

   /*=============================================*/
   /* Change the current module to the specified  */
   /* module and save the previous current module */
   /* for the return value.                       */
   /*=============================================*/

   rv = (void *) DefmoduleData(theEnv)->CurrentModule;
   DefmoduleData(theEnv)->CurrentModule = newValue;

   /*==========================================================*/
   /* Call the list of registered functions that need to know  */
   /* when the module has changed. The module change functions */
   /* should only be called if this is a "real" module change. */
   /* Many routines temporarily change the module to look for  */
   /* constructs, etc. The SaveCurrentModule function will     */
   /* disable the change functions from being called.          */
   /*==========================================================*/

   if (DefmoduleData(theEnv)->CallModuleChangeFunctions)
     {
      DefmoduleData(theEnv)->ModuleChangeIndex++;
      changeFunctions = DefmoduleData(theEnv)->AfterModuleChangeFunctions;
      while (changeFunctions != NULL)
        {
         (* (void (*)(void *)) changeFunctions->func)(theEnv);
         changeFunctions = changeFunctions->next;
        }
     }

   /*=====================================*/
   /* Return the previous current module. */
   /*=====================================*/

   return(rv);
  }

/********************************************************/
/* SaveCurrentModule: Saves current module on stack and */
/*   prevents SetCurrentModule() from calling change    */
/*   functions                                          */
/********************************************************/
globle void SaveCurrentModule(
  void *theEnv)
  {
   MODULE_STACK_ITEM *tmp;

   tmp = get_struct(theEnv,moduleStackItem);
   tmp->changeFlag = DefmoduleData(theEnv)->CallModuleChangeFunctions;
   DefmoduleData(theEnv)->CallModuleChangeFunctions = FALSE;
   tmp->theModule = DefmoduleData(theEnv)->CurrentModule;
   tmp->next = DefmoduleData(theEnv)->ModuleStack;
   DefmoduleData(theEnv)->ModuleStack = tmp;
  }

/**********************************************************/
/* RestoreCurrentModule: Restores saved module and resets */
/*   ability of SetCurrentModule() to call changed        */
/*   functions to previous state                          */
/**********************************************************/
globle void RestoreCurrentModule(
  void *theEnv)
  {
   MODULE_STACK_ITEM *tmp;

   tmp = DefmoduleData(theEnv)->ModuleStack;
   DefmoduleData(theEnv)->ModuleStack = tmp->next;
   DefmoduleData(theEnv)->CallModuleChangeFunctions = tmp->changeFlag;
   DefmoduleData(theEnv)->CurrentModule = tmp->theModule;
   rtn_struct(theEnv,moduleStackItem,tmp);
  }

/*************************************************************/
/* GetModuleItem: Returns the data pointer for the specified */
/*   module item in the specified module. If no module is    */
/*   indicated, then the module item for the current module  */
/*   is returned.                                            */
/*************************************************************/
globle void *GetModuleItem(
  void *theEnv,
  struct defmodule *theModule,
  int moduleItemIndex)
  {
   if (theModule == NULL)
     {
      if (DefmoduleData(theEnv)->CurrentModule == NULL) return(NULL);
      theModule = DefmoduleData(theEnv)->CurrentModule;
     }

   if (theModule->itemsArray == NULL) return (NULL);
   return ((void *) theModule->itemsArray[moduleItemIndex]);
  }

/************************************************************/
/* SetModuleItem: Sets the data pointer for the specified   */
/*   module item in the specified module. If no module is   */
/*   indicated, then the module item for the current module */
/*   is returned.                                           */
/************************************************************/
globle void SetModuleItem(
  void *theEnv,
  struct defmodule *theModule,
  int moduleItemIndex,
  void *newValue)
  {
   if (theModule == NULL)
     {
      if (DefmoduleData(theEnv)->CurrentModule == NULL) return;
      theModule = DefmoduleData(theEnv)->CurrentModule;
     }

   if (theModule->itemsArray == NULL) return;
   theModule->itemsArray[moduleItemIndex] = (struct defmoduleItemHeader *) newValue;
  }

/******************************************************/
/* CreateMainModule: Creates the default MAIN module. */
/******************************************************/
globle void CreateMainModule(
  void *theEnv)
  {
   struct defmodule *newDefmodule;
   struct moduleItem *theItem;
   int i;
   struct defmoduleItemHeader *theHeader;

   /*=======================================*/
   /* Allocate the defmodule data structure */
   /* and name it the MAIN module.          */
   /*=======================================*/

   newDefmodule = get_struct(theEnv,defmodule);
   newDefmodule->name = (SYMBOL_HN *) EnvAddSymbol(theEnv,"MAIN");
   IncrementSymbolCount(newDefmodule->name);
   newDefmodule->next = NULL;
   newDefmodule->ppForm = NULL;
   newDefmodule->importList = NULL;
   newDefmodule->exportList = NULL;
   newDefmodule->bsaveID = 0L;
   newDefmodule->usrData = NULL;

   /*==================================*/
   /* Initialize the array for storing */
   /* the module's construct lists.    */
   /*==================================*/

   if (DefmoduleData(theEnv)->NumberOfModuleItems == 0) newDefmodule->itemsArray = NULL;
   else
     {
      newDefmodule->itemsArray = (struct defmoduleItemHeader **)
                                 gm2(theEnv,sizeof(void *) * DefmoduleData(theEnv)->NumberOfModuleItems);
      for (i = 0, theItem = DefmoduleData(theEnv)->ListOfModuleItems;
           (i < DefmoduleData(theEnv)->NumberOfModuleItems) && (theItem != NULL);
           i++, theItem = theItem->next)
        {
         if (theItem->allocateFunction == NULL)
           { newDefmodule->itemsArray[i] = NULL; }
         else
           {
            newDefmodule->itemsArray[i] = (struct defmoduleItemHeader *)
                                          (*theItem->allocateFunction)(theEnv);
            theHeader = (struct defmoduleItemHeader *) newDefmodule->itemsArray[i];
            theHeader->theModule = newDefmodule;
            theHeader->firstItem = NULL;
            theHeader->lastItem = NULL;
           }
        }
     }

   /*=======================================*/
   /* Add the module to the list of modules */
   /* and make it the current module.       */
   /*=======================================*/

#if (! BLOAD_ONLY) && (! RUN_TIME) && DEFMODULE_CONSTRUCT
   SetNumberOfDefmodules(theEnv,1L);
#endif

   DefmoduleData(theEnv)->LastDefmodule = newDefmodule;
   DefmoduleData(theEnv)->ListOfDefmodules = newDefmodule;
   EnvSetCurrentModule(theEnv,(void *) newDefmodule);
  }

/*********************************************************************/
/* SetListOfDefmodules: Sets the list of defmodules to the specified */
/*   value. Normally used when initializing a run-time module or     */
/*   when bloading a binary file to install the list of defmodules.  */
/*********************************************************************/
globle void SetListOfDefmodules(
  void *theEnv,
  void *defmodulePtr)
  {
   DefmoduleData(theEnv)->ListOfDefmodules = (struct defmodule *) defmodulePtr;

   DefmoduleData(theEnv)->LastDefmodule = DefmoduleData(theEnv)->ListOfDefmodules;
   if (DefmoduleData(theEnv)->LastDefmodule == NULL) return;
   while (DefmoduleData(theEnv)->LastDefmodule->next != NULL) DefmoduleData(theEnv)->LastDefmodule = DefmoduleData(theEnv)->LastDefmodule->next;
  }

/********************************************************************/
/* EnvGetNextDefmodule: If passed a NULL pointer, returns the first */
/*   defmodule in the ListOfDefmodules. Otherwise returns the next  */
/*   defmodule following the defmodule passed as an argument.       */
/********************************************************************/
globle void *EnvGetNextDefmodule(
  void *theEnv,
  void *defmodulePtr)
  {
   if (defmodulePtr == NULL)
     { return((void *) DefmoduleData(theEnv)->ListOfDefmodules); }
   else
     { return((void *) (((struct defmodule *) defmodulePtr)->next)); }
  }

/*****************************************/
/* EnvGetDefmoduleName: Returns the name */
/*   of the specified defmodule.         */
/*****************************************/
#if IBM_TBC
#pragma argsused
#endif
globle char *EnvGetDefmoduleName(
  void *theEnv,
  void *defmodulePtr)
  { 
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   return(ValueToString(((struct defmodule *) defmodulePtr)->name)); 
  }

/***************************************************/
/* EnvGetDefmodulePPForm: Returns the pretty print */
/*   representation of the specified defmodule.    */
/***************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle char *EnvGetDefmodulePPForm(
  void *theEnv,
  void *defmodulePtr)
  { 
#if MAC_MCW || IBM_MCW
#pragma unused(theEnv)
#endif

   return(((struct defmodule *) defmodulePtr)->ppForm); 
  }

#if (! RUN_TIME)

/***********************************************/
/* RemoveAllDefmodules: Removes all defmodules */
/*   from the current environment.             */
/***********************************************/
globle void RemoveAllDefmodules(
  void *theEnv)
  {
   struct defmodule *nextDefmodule;

   while (DefmoduleData(theEnv)->ListOfDefmodules != NULL)
     {
      nextDefmodule = DefmoduleData(theEnv)->ListOfDefmodules->next;
      ReturnDefmodule(theEnv,DefmoduleData(theEnv)->ListOfDefmodules,FALSE);
      DefmoduleData(theEnv)->ListOfDefmodules = nextDefmodule;
     }

   DefmoduleData(theEnv)->CurrentModule = NULL;
   DefmoduleData(theEnv)->LastDefmodule = NULL;
  }

/************************************************************/
/* ReturnDefmodule: Returns the data structures associated  */
/*   with a defmodule construct to the pool of free memory. */
/************************************************************/
static void ReturnDefmodule(
  void *theEnv,
  struct defmodule *theDefmodule,
  BOOLEAN environmentClear)
  {
   int i;
   struct moduleItem *theItem;
   struct portItem *theSpec, *nextSpec;

   /*=====================================================*/
   /* Set the current module to the module being deleted. */
   /*=====================================================*/

   if (theDefmodule == NULL) return;
   
   if (! environmentClear)
     { EnvSetCurrentModule(theEnv,(void *) theDefmodule); }

   /*============================================*/
   /* Call the free functions for the constructs */
   /* belonging to this module.                  */
   /*============================================*/

   if (theDefmodule->itemsArray != NULL)
     {
      if (! environmentClear)
        {
         for (i = 0, theItem = DefmoduleData(theEnv)->ListOfModuleItems;
              (i < DefmoduleData(theEnv)->NumberOfModuleItems) && (theItem != NULL);
              i++, theItem = theItem->next)
           {
            if (theItem->freeFunction != NULL)
              { (*theItem->freeFunction)(theEnv,theDefmodule->itemsArray[i]); }
           }
        }

      rm(theEnv,theDefmodule->itemsArray,sizeof(void *) * DefmoduleData(theEnv)->NumberOfModuleItems);
    }

   /*======================================================*/
   /* Decrement the symbol count for the defmodule's name. */
   /*======================================================*/

   if (! environmentClear)
     { DecrementSymbolCount(theEnv,theDefmodule->name); }

   /*====================================*/
   /* Free the items in the import list. */
   /*====================================*/

   theSpec = theDefmodule->importList;
   while (theSpec != NULL)
     {
      nextSpec = theSpec->next;
      if (! environmentClear)
        {
         if (theSpec->moduleName != NULL) DecrementSymbolCount(theEnv,theSpec->moduleName);
         if (theSpec->constructType != NULL) DecrementSymbolCount(theEnv,theSpec->constructType);
         if (theSpec->constructName != NULL) DecrementSymbolCount(theEnv,theSpec->constructName);
        }
      rtn_struct(theEnv,portItem,theSpec);
      theSpec = nextSpec;
     }

   /*====================================*/
   /* Free the items in the export list. */
   /*====================================*/

   theSpec = theDefmodule->exportList;
   while (theSpec != NULL)
     {
      nextSpec = theSpec->next;
      if (! environmentClear)
        {
         if (theSpec->moduleName != NULL) DecrementSymbolCount(theEnv,theSpec->moduleName);
         if (theSpec->constructType != NULL) DecrementSymbolCount(theEnv,theSpec->constructType);
         if (theSpec->constructName != NULL) DecrementSymbolCount(theEnv,theSpec->constructName);
        }
      rtn_struct(theEnv,portItem,theSpec);
      theSpec = nextSpec;
     }

   /*=========================================*/
   /* Free the defmodule pretty print string. */
   /*=========================================*/

   if (theDefmodule->ppForm != NULL)
     {
      rm(theEnv,theDefmodule->ppForm,
         (int) sizeof(char) * (strlen(theDefmodule->ppForm) + 1));
     }
     
   /*=======================*/
   /* Return the user data. */
   /*=======================*/

   ClearUserDataList(theEnv,theDefmodule->usrData);
   
   /*======================================*/
   /* Return the defmodule data structure. */
   /*======================================*/

   rtn_struct(theEnv,defmodule,theDefmodule);
  }

#endif /* (! RUN_TIME) */

/**********************************************************************/
/* EnvFindDefmodule: Searches for a defmodule in the list of defmodules. */
/*   Returns a pointer to the defmodule if found, otherwise NULL.     */
/**********************************************************************/
globle void *EnvFindDefmodule(
  void *theEnv,
  char *defmoduleName)
  {
   struct defmodule *defmodulePtr;
   SYMBOL_HN *findValue;

   if ((findValue = (SYMBOL_HN *) FindSymbolHN(theEnv,defmoduleName)) == NULL) return(NULL);

   defmodulePtr = DefmoduleData(theEnv)->ListOfDefmodules;
   while (defmodulePtr != NULL)
     {
      if (defmodulePtr->name == findValue)
        { return((void *) defmodulePtr); }

      defmodulePtr = defmodulePtr->next;
     }

   return(NULL);
  }

/*************************************************/
/* GetCurrentModuleCommand: H/L access routine   */
/*   for the get-current-module command.         */
/*************************************************/
globle SYMBOL_HN *GetCurrentModuleCommand(
  void *theEnv)
  {
   struct defmodule *theModule;

   EnvArgCountCheck(theEnv,"get-current-module",EXACTLY,0);

   theModule = (struct defmodule *) EnvGetCurrentModule(theEnv);

   if (theModule == NULL) return((SYMBOL_HN *) SymbolData(theEnv)->FalseSymbol);

   return((SYMBOL_HN *) EnvAddSymbol(theEnv,ValueToString(theModule->name)));
  }

/*************************************************/
/* SetCurrentModuleCommand: H/L access routine   */
/*   for the set-current-module command.         */
/*************************************************/
globle SYMBOL_HN *SetCurrentModuleCommand(
  void *theEnv)
  {
   DATA_OBJECT argPtr;
   char *argument;
   struct defmodule *theModule;
   SYMBOL_HN *defaultReturn;

   /*=====================================================*/
   /* Check for the correct number and type of arguments. */
   /*=====================================================*/

   theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
   if (theModule == NULL) return((SYMBOL_HN *) SymbolData(theEnv)->FalseSymbol);

   defaultReturn = (SYMBOL_HN *) EnvAddSymbol(theEnv,ValueToString(((struct defmodule *) EnvGetCurrentModule(theEnv))->name));

   if (EnvArgCountCheck(theEnv,"set-current-module",EXACTLY,1) == -1)
     { return(defaultReturn); }

   if (EnvArgTypeCheck(theEnv,"set-current-module",1,SYMBOL,&argPtr) == FALSE)
     { return(defaultReturn); }

   argument = DOToString(argPtr);

   /*================================================*/
   /* Set the current module to the specified value. */
   /*================================================*/

   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,argument);

   if (theModule == NULL)
     {
      CantFindItemErrorMessage(theEnv,"defmodule",argument);
      return(defaultReturn);
     }

   EnvSetCurrentModule(theEnv,(void *) theModule);

   /*================================*/
   /* Return the new current module. */
   /*================================*/

   return((SYMBOL_HN *) defaultReturn);
  }

/*************************************************/
/* AddAfterModuleChangeFunction: Adds a function */
/*   to the list of functions to be called after */
/*   a module change occurs.                     */
/*************************************************/
globle void AddAfterModuleChangeFunction(
  void *theEnv,
  char *name,
  void (*func)(void *),
  int priority)
  {
   DefmoduleData(theEnv)->AfterModuleChangeFunctions =
     AddFunctionToCallList(theEnv,name,priority,func,DefmoduleData(theEnv)->AfterModuleChangeFunctions,TRUE);
  }

/************************************************/
/* IllegalModuleSpecifierMessage: Error message */
/*   for the illegal use of a module specifier. */
/************************************************/
globle void IllegalModuleSpecifierMessage(
  void *theEnv)
  {
   PrintErrorID(theEnv,"MODULDEF",1,TRUE);
   EnvPrintRouter(theEnv,WERROR,"Illegal use of the module specifier.\n");
  }

