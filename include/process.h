
// sys/ptrace.h
#define	PT_TRACE_ME		0	/* child declares it's being traced */
#define	PT_READ_I		1	/* read word in child's I space */
#define	PT_READ_D		2	/* read word in child's D space */
/* was	PT_READ_U		3	 * read word in child's user structure */
#define	PT_WRITE_I		4	/* write word in child's I space */
#define	PT_WRITE_D		5	/* write word in child's D space */
/* was	PT_WRITE_U		6	 * write word in child's user structure */
#define	PT_CONTINUE		7	/* continue the child */
#define	PT_KILL			8	/* kill the child process */
#define	PT_STEP			9	/* single step the child */
#define	PT_ATTACH		10	/* trace some running process */
#define	PT_DETACH		11	/* stop tracing a process */
#define PT_IO			12	/* do I/O to/from stopped process. */
#define	PT_LWPINFO		13	/* Info about the LWP that stopped. */
#define PT_GETNUMLWPS	14	/* get total number of threads */
#define PT_GETLWPLIST	15	/* get thread list */
#define PT_CLEARSTEP	16	/* turn off single step */
#define PT_SETSTEP		17	/* turn on single step */
#define PT_SUSPEND		18	/* suspend a thread */
#define PT_RESUME		19	/* resume a thread */
#define	PT_TO_SCE		20
#define	PT_TO_SCX		21
#define	PT_SYSCALL		22
#define	PT_FOLLOW_FORK	23
#define PT_GETREGS      33	/* get general-purpose registers */
#define PT_SETREGS      34	/* set general-purpose registers */
#define PT_GETFPREGS    35	/* get floating-point registers */
#define PT_SETFPREGS    36	/* set floating-point registers */
#define PT_GETDBREGS    37	/* get debugging registers */
#define PT_SETDBREGS    38	/* set debugging registers */
#define	PT_VM_TIMESTAMP	40	/* Get VM version (timestamp) */
#define	PT_VM_ENTRY		41	/* Get VM map (entry) */

#define PIOD_READ_D		1	/* Read from D space */
#define PIOD_WRITE_D	2	/* Write to D space */
#define PIOD_READ_I		3	/* Read from I space */
#define PIOD_WRITE_I	4	/* Write to I space */

// sys/wait.h
#define	WNOHANG		1	/* Don't hang in wait. */
#define	WUNTRACED	2	/* Tell about stopped, untraced children. */
#define	WSTOPPED	WUNTRACED   /* SUS compatibility */
#define	WCONTINUED	4	/* Report a job control continued process. */
#define	WNOWAIT	8	/* Poll only. Don't delete the proc entry. */

/* Argument structure for PT_VM_ENTRY. */
struct ptrace_vm_entry {
	int		pve_entry;	/* Entry number used for iteration. */
	int		pve_timestamp;	/* Generation number of VM map. */
	unsigned long		pve_start;	/* Start VA of range. */
	unsigned long		pve_end;	/* End VA of range (incl). */
	unsigned long		pve_offset;	/* Offset in backing object. */
	unsigned int		pve_prot;	/* Protection of memory range. */
	unsigned int		pve_pathlen;	/* Size of path. */
	long		pve_fileid;	/* File ID. */
	uint32_t	pve_fsid;	/* File system ID. */
	char		*pve_path;	/* Path name of object. */
};

struct rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long	ru_maxrss;		/* max resident set size */
#define	ru_first	ru_ixrss
	long	ru_ixrss;		/* integral shared memory size */
	long	ru_idrss;		/* integral unshared data " */
	long	ru_isrss;		/* integral unshared stack " */
	long	ru_minflt;		/* page reclaims */
	long	ru_majflt;		/* page faults */
	long	ru_nswap;		/* swaps */
	long	ru_inblock;		/* block input operations */
	long	ru_oublock;		/* block output operations */
	long	ru_msgsnd;		/* messages sent */
	long	ru_msgrcv;		/* messages received */
	long	ru_nsignals;		/* signals received */
	long	ru_nvcsw;		/* voluntary context switches */
	long	ru_nivcsw;		/* involuntary " */
#define	ru_last		ru_nivcsw
};


//machine/reg.h
typedef long int register_t;
struct reg {
	register_t	r_r15;
	register_t	r_r14;
	register_t	r_r13;
	register_t	r_r12;
	register_t	r_r11;
	register_t	r_r10;
	register_t	r_r9;
	register_t	r_r8;
	register_t	r_rdi;
	register_t	r_rsi;
	register_t	r_rbp;
	register_t	r_rbx;
	register_t	r_rdx;
	register_t	r_rcx;
	register_t	r_rax;
	uint32_t	r_trapno;
	uint16_t	r_fs;
	uint16_t	r_gs;
	uint32_t	r_err;
	uint16_t	r_es;
	uint16_t	r_ds;
	register_t	r_rip;
	register_t	r_cs;
	register_t	r_rflags;
	register_t	r_rsp;
	register_t	r_ss;
};
struct __ptrace_io_desc {
	int	piod_op;		/* I/O operation */
	void *piod_offs;	/* child offset */
	void *piod_addr;	/* parent offset */
	size_t piod_len;	/* request length */
};
int getAllProcess(const char* targetProcess, int (*callBack)(char* pname, int pid))
{
	int names[] = {1, 14, 0};
	int namesLength = sizeof(names) / sizeof(names[0]);
	unsigned long int lengthOfOldValue;
	int sysCtlReturn;
	sysCtlReturn = sysctl(names, namesLength, NULL, &lengthOfOldValue, NULL, NULL);

	if (sysCtlReturn == -1)
		return -1;

	char* data = (char*)malloc(lengthOfOldValue);
	sysCtlReturn = sysctl(names, namesLength, data, &lengthOfOldValue, NULL, NULL);
	if (sysCtlReturn == -1)
	{
		free(data);
		return -1;
	}
	int structSize = 1096;
	int procCount = lengthOfOldValue / structSize;
	int skipDupProcess = 0;

	for(int i = 0; i < procCount; i++)
	{
		void* processStructure = (data + (structSize * i));
		int pid = *(int*)(processStructure + 72);
		if (skipDupProcess == pid)
			continue;
		skipDupProcess = pid;
		char* name = (char*)(processStructure + 447);
        if ((targetProcess != NULL && strcmp(targetProcess, name) == 0))
            return pid;
		if (callBack != NULL && callBack(name, pid))
            return pid;
	}
	free(data);
	return -1;
}
int getProcess(const char* procName)
{
    return getAllProcess(procName, NULL);
}
int getProcessName(int pid, char* dist){
	int names[] = {1, 14, 1, pid}; //nametomib kern.proc.pid
	int namesLength = sizeof(names) / sizeof(names[0]);
	unsigned long int lengthOfOldValue;
	void* dump = malloc(lengthOfOldValue);
	int sysCtlReturn = sysctl(names, namesLength, dump, &lengthOfOldValue, NULL, NULL);
	if (sysCtlReturn == -1)
		return -1;
	char* procName = (char*)(dump + 0x1BF);
	strcpy(dist, procName);
	free(dump);
	return sysCtlReturn;
}
int wait4(int pid, int *status, int options, struct	rusage *rusage){
	return syscall(7, pid, status, options, rusage);
}
int __ptrace(int req, int pid, void* addr, int data) {
	return syscall(26, req, pid, addr, data);
}
int processSingleStep(int pid){
	return __ptrace(PT_STEP, pid, NULL, NULL);
}
int processClearStep(int pid){
	return __ptrace(PT_CLEARSTEP, pid, NULL, NULL);
}
int processContinue(int pid, void* address){
	return __ptrace(PT_CONTINUE, pid, address, NULL);
}
int processSuspend(int pid){
	return __ptrace(PT_SUSPEND, pid, NULL, NULL);
}
int processResume(int pid){
	return __ptrace(PT_RESUME, pid, NULL, NULL);
}
int processKill(int pid)
{
	return __ptrace(PT_KILL, pid, NULL, NULL);
}
int processGetRegs(int pid, struct reg* rg){
	return __ptrace(PT_GETREGS, pid, (void*)rg, NULL);
}
int processDetach(int);

int processAttach(int pid)
{

	int res = __ptrace(PT_ATTACH, pid, NULL, NULL);
	if (res != 0)
		return res;
	int status = 0;
	wait4(pid, &status, WUNTRACED, NULL);
	//struct reg rg;
	//processGetRegs(pid, &rg);
	//processContinue(pid, (void*)rg.r_rip);
	return res;
}

int processDetach(int pid) {
	return __ptrace(PT_DETACH, pid, NULL, NULL);
}
int processgetVMTimeStamp(int pid){
	return __ptrace(PT_VM_TIMESTAMP, pid, NULL, NULL);
}
int getVMEntry(int pid, struct ptrace_vm_entry* entryStructure){
	return __ptrace(PT_VM_ENTRY, pid, entryStructure, NULL);
}
int processReadBytes(int pid, void* offset, void* buffer, size_t len) {
	struct __ptrace_io_desc pt_desc;
	pt_desc.piod_op = PIOD_READ_D;
	pt_desc.piod_addr = buffer;
	pt_desc.piod_offs = offset;
	pt_desc.piod_len = len;
	return __ptrace(PT_IO, pid, &pt_desc, NULL);
}
int processWriteBytes(int pid, void* offset, void *buffer, size_t len) {
	struct __ptrace_io_desc pt_desc;
	pt_desc.piod_op = PIOD_WRITE_D;
	pt_desc.piod_addr = buffer;
	pt_desc.piod_offs = offset;
	pt_desc.piod_len = len;
	return __ptrace(PT_IO, pid, &pt_desc, NULL);
}
int writeMemory(int pid, void* offset, void *buffer, size_t len)
{
    int res = processWriteBytes(pid, offset, buffer, len);
	return res;
}
int readMemory(int pid, void* offset, void *buffer, size_t len)
{
    int res = processReadBytes(pid, offset, buffer, len);
	return res;
}
