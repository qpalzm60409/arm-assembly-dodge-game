/* module to provide page tables
 *
 * RCS $Revision: 1.9.2.2 $
 * Checkin $Date: 1999/12/21 18:42:44 $
 * Revising $Author: mwilliam $
 */

#include <ctype.h>
#include "armdefs.h"
#include "armcnf.h"

enum access {
  NO_ACCESS,
  NO_USR_W,
  SVC_RW,
  ALL_ACCESS
  };

enum entrytype {
  INVALID,
  PAGE,
  SECTION
  };

#define U_BIT 16
#define C_BIT 8
#define B_BIT 4

#define L1Entry(type,addr,dom,ucb,acc) \
  ((type==SECTION) ? (((addr)&0xfff00000)|((acc)<<10)|  \
                      ((dom)<<5)|(ucb)|(type)) :        \
   (type==PAGE) ? (((addr)&0xfffffc00)|((dom)<<5)|      \
                   ((ucb)&U_BIT)|(type)) :              \
   0)

static struct {
  tag_t option;
  ARMword bit;
} ctl_flags[] = {
  ARMulCnf_MMU,         MMU_M,
  ARMulCnf_AlignFaults, MMU_A,
  ARMulCnf_Cache,       MMU_C,
  ARMulCnf_WriteBuffer, MMU_W,
  ARMulCnf_Prog32,      MMU_P,
  ARMulCnf_Data32,      MMU_D,
  ARMulCnf_LateAbort,   MMU_L,
  ARMulCnf_BigEnd,      MMU_B,
  ARMulCnf_SystemProt,  MMU_S,
  ARMulCnf_ROMProt,     MMU_R,
  ARMulCnf_BranchPredict, MMU_Z,
  ARMulCnf_ICache,      MMU_I,
  ARMulCnf_HighExceptionVectors, MMU_V
  };

typedef struct {
  toolconf clone;
  ARMul_State *state;
} pagetab;

static unsigned32 ControlRegister(toolconf config, unsigned32 value)
{
    unsigned int i;
    char const *option;

    option = ToolConf_Lookup(config, Dbg_Cnf_ByteSex);
    if (option != NULL) {
        switch (safe_toupper(option[0])) {
        case 'B':
            value |= MMU_B;
            break;
        case 'L':
            value &= ~MMU_B;
            break;
        }
    }

#if 0
    core_config = ARMul_SetConfig(state, 0, 0);
    
    /* These are the options that might already be set by the core */
    core_mask = (MMU_B | MMU_L | MMU_P | MMU_D | MMU_V | MMU_Z);
    value = (value & ~core_mask) | (core_config & core_mask);
#endif

    for (i = 0; i < sizeof(ctl_flags) / sizeof(ctl_flags[0]); i++) {
        option = ToolConf_Lookup(config, ctl_flags[i].option);
        if (option != NULL) {
            value = ToolConf_AddFlag(option, value,ctl_flags[i].bit, TRUE);
        }
    }

    return value;
}

static void MMU(pagetab *tab)
{
  ARMul_State *state = tab->state;
  toolconf config = tab->clone;
  const char *option;
  ARMword value;
  ARMword page;
  ARMword entry;
  unsigned int i;

  value=ToolConf_DLookupUInt(config, ARMulCnf_DAC, 0x3);
  ARMul_CPWrite(state,15,3,&value);
  
  value=ToolConf_DLookupUInt(config, ARMulCnf_PageTableBase, 0x40000000);
  value&=0xffffc000;            /* align */
  ARMul_CPWrite(state,15,2,&value);
  
  /* start with all pages flat, uncacheable, read/write, unbufferable */
  entry=L1Entry(SECTION,0,0,U_BIT|C_BIT|B_BIT,ALL_ACCESS);
  for (i=0, page=value; i<4096; i++, page+=4) {
    ARMul_WriteWord(state, page, entry | (i<<20));
  }

  /* look to see if any regions are defined on top of this */
  for (i=0; i<100; i++) {
    char buffer[32];
    ARMword low,physical,mask;
    ARMword page;
    toolconf region;
    int j,n;
      
    sprintf(buffer,"REGION[%d]",i);
    region=ToolConf_Child(config,(tag_t)buffer);
    if (region==NULL) break;    /* stop */

    option=ToolConf_Lookup(region,ARMulCnf_VirtualBase);
    if (option) {
      low=option ? strtoul(option,NULL,0) : 0;
      physical=ToolConf_DLookupUInt(region, ARMulCnf_PhysicalBase, low);
    } else {
      low=physical=ToolConf_DLookupUInt(region, ARMulCnf_PhysicalBase,0);
    }

    option = ToolConf_Lookup(region, ARMulCnf_Size);
    if (option != NULL) {
      unsigned long size = ToolConf_Power(option, TRUE);
      if (size == 0) {          /* assume 4GB */
        n = 4096;
      } else {
        n = size >> 20;
      }
    } else {
      /* backwards compatability */
      n = ToolConf_DLookupUInt(region, ARMulCnf_Pages, 4096);
    }

    mask=0;
    option=ToolConf_Lookup(region,ARMulCnf_Cacheable);
    mask=ToolConf_AddFlag(option,mask,C_BIT,TRUE);
    option=ToolConf_Lookup(region,ARMulCnf_Bufferable);
    mask=ToolConf_AddFlag(option,mask,B_BIT,TRUE);
    option=ToolConf_Lookup(region,ARMulCnf_Updateable);
    mask=ToolConf_AddFlag(option,mask,U_BIT,TRUE);

    mask |= (ToolConf_DLookupUInt(region, ARMulCnf_Domain, 0) & 0xf) << 5;
    mask |= (ToolConf_DLookupUInt(region, ARMulCnf_AccessPermissions,
                                  ALL_ACCESS) & 3) << 10;

    /* set TRANSLATE=NO to generate translation faults */
    if (ToolConf_DLookupBool(region, ARMulCnf_Translate, TRUE))
      mask |= SECTION;
    
    low&=0xfff00000;
    mask=(physical & 0xfff00000) | (mask & 0xdfe);
    
    j=low>>20;          /* index of first section */
    n+=j; if (n>4096) n=4096;
    for (page=(value+j*4); j<n; j++, mask+=0x100000, page+=4) {
      ARMul_WriteWord(state,page,mask);
    }
  }

  /* now enable the caches, etc. */
  value = ControlRegister(config, 0);

  if (value & (MMU_C | MMU_W)) {
      value |= MMU_M;           /* always enable the MMU too */
  }

  ARMul_CPWrite(state,15,1,&value); /* enable the MMU etc. */
}

static void ProtectionUnit(pagetab *tab, ARMword processor)
{
  /* /* set up cacheable etc. from toolconf */
  ARMul_State *state = tab->state;
  toolconf config = tab->clone;
  const char *option;


  if (processor & 0x40) { /* ARM940 */
    /* Variables to hold the values eventually sent to copro 15. 
       They are all different sizes, but for simplicity's sake they
       are stored as longs.
     */
    unsigned long Config_reg=0;
    unsigned long Cacheable_reg=0;
    unsigned long WriteBuffer_reg=0;
    unsigned long Protection_reg=0;
    unsigned long Region_regs[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    ARMword start;
    ARMword size;
    toolconf region;
    char buffer[32];
    int i, j, x;

    for (i = 0; i<3; i++){ /* For each type of region. Highest priority comes last and ovewrites */
        for (j = 0; j < 8; j++) {
            switch (i) {
            case 0 :
                sprintf(buffer,"IREGION[%d]",j);
                break;
            case 1 :
                sprintf(buffer,"DREGION[%d]",j);
                break;
            case 2 :
                sprintf(buffer,"REGION[%d]",j);
                break;
            }
            region=ToolConf_Child(config,(tag_t)buffer);
            if (region!=NULL) {
                /* Works for D and I side identically at the mo */

                start=ToolConf_DLookupUInt(region, ARMulCnf_PhysicalBase,0);

                option = ToolConf_Lookup(region, ARMulCnf_Size);
                if (option != NULL) {
                  size = ToolConf_Power(option, TRUE);
                } else {
                  int n = ToolConf_DLookupUInt(region, ARMulCnf_Pages, 4096);
                  size = n << 20;
                }

                /* to get the 5 bit size value from the actual size,
                 * we have to do a linear search, to find the most 
                 * significant bit set.(Any other bit is ignored)
                 */
                for (x = 31; x >= 12; x--) {
                    /* Any value for size less than 0x1000 is considered 0 i.e. 4G */
                    if (size & (1 << x)) break;
                }
                size = (x < 12) ? 0x1f : x-1;
                Region_regs[j] = 1 |(size << 1) | (start << 12); /* for the moment I am assuming if we get this far we want the region enabled */
                Region_regs[j+8] = 1 |(size << 1) | (start << 12);

                option=ToolConf_Lookup(region,ARMulCnf_Cacheable);
                Cacheable_reg = ToolConf_AddFlag(option,Cacheable_reg, 1<<j, TRUE);
                Cacheable_reg = ToolConf_AddFlag(option,Cacheable_reg, 1<<(j+8), TRUE);/* I side */
                option=ToolConf_Lookup(region,ARMulCnf_Bufferable);
                WriteBuffer_reg = ToolConf_AddFlag(option,WriteBuffer_reg, 1<<j, TRUE);

                Protection_reg |= (ToolConf_DLookupUInt(region, ARMulCnf_AccessPermissions, ALL_ACCESS) & 3 ) << (j*2);
                Protection_reg |= (ToolConf_DLookupUInt(region, ARMulCnf_AccessPermissions, ALL_ACCESS) & 3 ) << ((j*2) + 16);

            }
        }
    }
    /* Now write all the values to the copro */
    ARMul_CPWrite(state, 15, 2, &Cacheable_reg);
    ARMul_CPWrite(state, 15, 3, &WriteBuffer_reg);
    ARMul_CPWrite(state, 15, 5, &Protection_reg);
    ARMul_CPWrite(state, 15, 6, Region_regs);

    /* Final thing to do is set the Configuration register */
    ARMul_CPRead(state, 15, 1, &Config_reg);
    Config_reg = ControlRegister(config, Config_reg);

    if (Config_reg & (MMU_C | MMU_I)) {
        Config_reg |= MMU_M;    /* always enable the PU too */
    }

    ARMul_CPWrite(state, 15, 1, &Config_reg);
  }

  return;
}

static void InterruptHandler(void *handle,unsigned int which)
{
  pagetab *tab = (pagetab *)handle;
  ARMul_State *state = tab->state;
  ARMword processor;
  unsigned int const *cp_bytes;

  if (!(which & ARMul_InterruptUpcallReset))
    return;                     /* we only care about reset */

  /* Decide whether we have an MMU or a protection unit */
  cp_bytes = ARMul_CPRegBytes(state, 15);
  if (cp_bytes == NULL || cp_bytes[0] == 0 || cp_bytes[1] != sizeof(ARMword))
    /* there's no cp15, so no need to pagetables */
    return;

  (void)ARMul_CPRead(state, 15, 0, &processor);
  processor = (processor >> 4) & 0xfff;

  /* processor is the 'part number' of the processor, e.g.:
   * 0x61 -> ARM610, 0x710 -> ARM710, 0x940 -> ARM940
   */

  if ((processor & 0x0f0) == 0x040) { /* protection unit */
    ProtectionUnit(tab, processor);
  } else {
    /* else assume an MMU */
    MMU(tab);
  }

}

static void PagetabFree(void *handle)
{
  pagetab *tab = (pagetab *)handle;
  ToolConf_Reset(tab->clone);
  free(tab);
}

static ARMul_Error PagetabInit(ARMul_State *state, toolconf config)
{
  pagetab *tab;
 
  ARMul_PrettyPrint(state,", Pagetables");


  tab = (pagetab *)malloc(sizeof(*tab));
  if (tab != NULL) {
      char const *option;
      tab->clone = ToolConf_Clone(config);
      option = ToolConf_Lookup(config, Dbg_Cnf_ByteSex);
      if (option != NULL) {
          ToolConf_AddTagged(tab->clone, Dbg_Cnf_ByteSex, option);
      }
  }
  if (tab != NULL && tab->clone != NULL) {
    tab->state = state;
    ARMul_InstallInterruptHandler(state, InterruptHandler, tab);
    ARMul_InstallExitHandler(state, PagetabFree, tab);
    return ARMulErr_NoError;
  }
  return ARMul_RaiseError(state, ARMulErr_OutOfMemory);
}


const ARMul_ModelStub ARMul_Pagetable = {
  PagetabInit,
  (tag_t)"Pagetables"
  };
