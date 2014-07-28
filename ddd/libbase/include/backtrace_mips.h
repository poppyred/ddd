#ifndef BACKTRACE_MIPS_H
#define BACKTRACE_MIPS_H

#include <asm/sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_ABI32

/*
 * Keep this struct definition in sync with the sigcontext fragment
 * in arch/mips/tools/offset.c
 */
typedef struct sigcontext_mips
{
    unsigned int		sc_regmask;	/* Unused */
    unsigned int		sc_status;	/* Unused */
    unsigned long long	sc_pc;
    unsigned long long	sc_regs[32];
    unsigned long long	sc_fpregs[32];
    unsigned int		sc_acx;		/* Was sc_ownedfp */
    unsigned int		sc_fpc_csr;
    unsigned int		sc_fpc_eir;	/* Unused */
    unsigned int		sc_used_math;
    unsigned int		sc_dsp;		/* dsp status, was sc_ssflags */
    unsigned long long	sc_mdhi;
    unsigned long long	sc_mdlo;
    unsigned long		sc_hi1;		/* Was sc_cause */
    unsigned long		sc_lo1;		/* Was sc_badvaddr */
    unsigned long		sc_hi2;		/* Was sc_sigset[4] */
    unsigned long		sc_lo2;
    unsigned long		sc_hi3;
    unsigned long		sc_lo3;
} sigcontext_mips;

#endif /* _MIPS_SIM == _MIPS_SIM_ABI32 */

#if _MIPS_SIM == _MIPS_SIM_ABI64 || _MIPS_SIM == _MIPS_SIM_NABI32

#include <linux/posix_types.h>
/*
 * Keep this struct definition in sync with the sigcontext fragment
 * in arch/mips/tools/offset.c
 *
 * Warning: this structure illdefined with sc_badvaddr being just an unsigned
 * int so it was changed to unsigned long in 2.6.0-test1.  This may break
 * binary compatibility - no prisoners.
 * DSP ASE in 2.6.12-rc4.  Turn sc_mdhi and sc_mdlo into an array of four
 * entries, add sc_dsp and sc_reserved for padding.  No prisoners.
 */
typedef struct sigcontext_mips
{
    __u64	sc_regs[32];
    __u64	sc_fpregs[32];
    __u64	sc_mdhi;
    __u64	sc_hi1;
    __u64	sc_hi2;
    __u64	sc_hi3;
    __u64	sc_mdlo;
    __u64	sc_lo1;
    __u64	sc_lo2;
    __u64	sc_lo3;
    __u64	sc_pc;
    __u32	sc_fpc_csr;
    __u32	sc_used_math;
    __u32	sc_dsp;
    __u32	sc_reserved;
} sigcontext_mips;

#ifdef __KERNEL__

typedef struct sigcontext32_mips
{
    __u32		sc_regmask;	/* Unused */
    __u32		sc_status;	/* Unused */
    __u64		sc_pc;
    __u64		sc_regs[32];
    __u64		sc_fpregs[32];
    __u32		sc_acx;		/* Only MIPS32; was sc_ownedfp */
    __u32		sc_fpc_csr;
    __u32		sc_fpc_eir;	/* Unused */
    __u32		sc_used_math;
    __u32		sc_dsp;		/* dsp status, was sc_ssflags */
    __u64		sc_mdhi;
    __u64		sc_mdlo;
    __u32		sc_hi1;		/* Was sc_cause */
    __u32		sc_lo1;		/* Was sc_badvaddr */
    __u32		sc_hi2;		/* Was sc_sigset[4] */
    __u32		sc_lo2;
    __u32		sc_hi3;
    __u32		sc_lo3;
} sigcontext32_mips;
#endif /* __KERNEL__ */

#endif /* _MIPS_SIM == _MIPS_SIM_ABI64 || _MIPS_SIM == _MIPS_SIM_NABI32 */

typedef struct ucontext_mips
{
    unsigned long	  uc_flags;
    struct ucontext_mips * uc_link;
    stack_t		  uc_stack;
    struct sigcontext_mips uc_mcontext;
    sigset_t	  uc_sigmask;	/* mask last for extensibility */
} ucontext_mips;

//add by liujiang for mips backtrace, 2011-12-10
//通过获得中断时pc指向的指令地址，向上遍历，以得到函数返回地址
//这样一步一步向上回溯，从而得到调用堆栈

//记录程序申请的内存空间
typedef struct pmap_line
{
    unsigned long vm_start;     /* Our start address within vm_mm. */
    unsigned long vm_end;       /* The first byte after our
                                   end address within vm_mm. */
    char perm[5];               /* permission */
    unsigned long vm_pgoff;     /* Offset (within vm_file) in PAGE_SIZE units,
                                   *not* PAGE_CACHE_SIZE */
    char dev[6];                /* device name */
    unsigned long ino;              /* innode number */
    struct pmap_line * next;
} pmap_line_t;


static void free_pmap_line(pmap_line_t * pmap_line_head)
{
    pmap_line_t * line = NULL;

    while ((line = pmap_line_head) != NULL)
    {
        pmap_line_head = pmap_line_head->next;
        free(line);
    }
}

static pmap_line_t * getpmaps(size_t pid)
{

    FILE * f;
    char buf[4096 + 100] = {0};
    pmap_line_t * pmap_line_tail = NULL;
    pmap_line_t * pmap_line_head = NULL;
    pmap_line_t * line = NULL;
    char fname [50] = {0};

    sprintf(fname, "/proc/%ld/maps", (long)pid);
    f = fopen(fname, "r");

    if (!f)
    {
        printf("open maps of %d error!\n", pid);
        return NULL;
    }

    while (!feof(f))
    {
        if (fgets(buf, sizeof(buf), f) == 0)
        {
            break;
        }

        line = (pmap_line_t *)malloc(sizeof(pmap_line_t));

        if (!line)
        {
            printf("getmaps failed!line =NULL.\n");
            free_pmap_line(pmap_line_head);
            fclose(f);
            return NULL;
        }

        memset(line, sizeof(pmap_line_t), 0);

        sscanf(buf, "%lx-%lx %4s %lx %5s %lu", &line->vm_start, &line->vm_end,
               line->perm, &line->vm_pgoff, line->dev, &line->ino);
        line->next = NULL;

        if (!pmap_line_head)
        {
            pmap_line_head = line;
        }

        if (pmap_line_tail)
        {
            pmap_line_tail->next = line;
        }

        pmap_line_tail = line;
    }

    fclose(f);
    return pmap_line_head;
}

//检测addr指向的地址能否访问
static int canReadAddr(unsigned long addr, pmap_line_t * pmap_line_head)
{
    pmap_line_t * line = pmap_line_head;


    if (!pmap_line_head)
    {
        return 0;
    }

    while (line)
    {
        if (line->perm[0] == 'r' &&
            addr >= line->vm_start && addr <= line->vm_end)
        {
            return 1;
        }

        line = line->next;
    }

    printf("cannot read address %p\n", (unsigned long *)addr);
    return 0;

}

/**PROC+**********************************************************************/
/* Name:      backtrace_mips                                              */
/*                                                                           */
/* Purpose:   standard backtrace function provided by glic does NOT support  */
/*            MIPS architecture. This function provids same functionality for*/
/*            MIPS, retreive the calling function stack pointer address based*/
/*            on current PC.                                                 */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:    IN/OUT     buffer   - buffer to hold text address returned     */
/*            IN/OUT     buffer   - buffer to hold text function address     */
/*                                  returned                                 */
/*                                                                           */
/*            IN         size     - buffer size                              */
/*                                                                           */
/*            IN         uc       -MIPS ucontext structure returned from     */
/*                                 kernel, holding registers when signal     */
/*                                 occurs.                                   */
/*                                                                           */
/**PROC-**********************************************************************/
static int backtrace_mips(void ** func,

                          int size, struct ucontext_mips * uc)
{
    unsigned long * addr = NULL;
    unsigned long * pc = NULL;
    unsigned long * ra = NULL;
    unsigned long * sp = NULL;
    size_t ra_offset = 0;
    size_t stack_size = 0;
    size_t depth = 0;
    int first = 0;


    if (size == 0)
    {
        return 0;
    }

    if (size < 0 || !uc)
    {
        return 0;
    }

    //get pmaps
    pmap_line_t * pmap_line_head = NULL;
    pmap_line_head = getpmaps(getpid());

    if (!pmap_line_head)
    {
        printf("pmap_line_head is NULL!\n");
        return 0;
    }

    //get current $pc, $ra and $sp
    pc = (unsigned long *)(unsigned long)uc->uc_mcontext.sc_pc;
    ra = (unsigned long *)(unsigned long)(uc->uc_mcontext.sc_regs[31]);
    sp = (unsigned long *)(unsigned long)(uc->uc_mcontext.sc_regs[29]);

    if (canReadAddr((unsigned long)pc, pmap_line_head))
    {
        depth = 1;
    }

    else
    {
        free_pmap_line(pmap_line_head);
        return 0;
    }

    //scanning to find the size of the current stack-frame
    ra_offset = stack_size = 0;

    func[0] = pc;
    addr = pc;
    first = 1;

    while (1)
    {
        if (!canReadAddr((unsigned long)addr, pmap_line_head))
        {
            free_pmap_line(pmap_line_head);
            return depth;
        }

        if (*addr == 0x1000ffff)
        {
            free_pmap_line(pmap_line_head);
            return depth;
        }

        switch ((*addr) & 0xffff0000)
        {
        case 0x23bd0000:

            /* 0x23bdxxxx: ADDI SP, SP, xxxx
             ADDIU -- Add immediate (with overflow)
              Description:
               Adds a register and a sign-extended immediate value and
               stores the result in a register

              Operation:
               $t = $s + imm; advance_pc (4);

              Syntax:
               addi $t, $s, imm

              Encoding:
               0010 00ss ssst tttt iiii iiii iiii iiii

              register : $29 sp

            */
        case 0x27bd0000:
            /* 0x27bdxxxx: ADDIU SP, SP, xxxx
             ADDIU -- Add immediate unsigned (no overflow)
              Description:
               Adds a register and a sign-extended immediate value and
               stores the result in a register

              Operation:
               $t = $s + imm; advance_pc (4);

              Syntax:
               addiu $t, $s, imm

              Encoding:
               0010 01ss ssst tttt iiii iiii iiii iiii

              register : $29 sp

            */
            stack_size = abs((short)((*addr) & 0xffff));

            if (!first && (!ra_offset || !stack_size))
            {
                //there's no return address or stack size,
                //reach the calling stack top
                free_pmap_line(pmap_line_head);
                return depth;
            }
            else
            {

                if ((int)depth == size)
                {
                    free_pmap_line(pmap_line_head);
                    return depth;
                }

                if (ra_offset && !canReadAddr((unsigned  long)sp + ra_offset, pmap_line_head))
                {
                    free_pmap_line(pmap_line_head);
                    return depth;
                }

                if (ra_offset)
                {
                    ra = (unsigned long *)(*(unsigned long *)
                                           ((unsigned long)sp + ra_offset));
                }

                if (stack_size)
                {
                    sp = (unsigned long *)
                         ((unsigned long)sp + stack_size);
                }

                func[depth++] = ra;
                stack_size = 0;
                ra_offset = 0;
                addr = ra;
            }

            first = 0;
            break;

        case 0xafbf0000:
            /*0xafbfxxxx : sw ra ,xxxx(sp)
              SW -- Store word
              Description:
               The contents of $t is stored at the specified address.

              Operation:
               MEM[$s + offset] = $t; advance_pc (4);

              Syntax:
               sw $t, offset($s)

              Encoding:
               1010 11ss ssst tttt iiii iiii iiii iiii

               register $31 : ra ; $29 : sp

            */

        case 0xffbf0000:
            /*0xffbfxxxx : sd ra ,xxxx(sp)
              SD-- Store double word
              Description:
               The contents of $t is stored at the specified address.

              Operation:
               MEM[$s + offset] = $t; advance_pc (4);

              Syntax:
               sd $t, offset($s)

              Encoding:
               1010 11ss ssst tttt iiii iiii iiii iiii

               register $31 : ra ; $29 : sp

            */
            ra_offset = (short)(*addr & 0xffff);
            --addr;
            break;

        default:
            --addr;
        }
    }

    free_pmap_line(pmap_line_head);
    return depth;
}
//end add

#endif
