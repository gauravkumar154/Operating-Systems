#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "fs.h"
#include "spinlock.h"


#define N_SWAP_SLOTS 8
#define PGSIZE 4096

struct swap_slot{
  uint is_free ;
  uint page_perm ;
  uint block_no ;
};

struct swap_slot swap_slots[N_SWAP_SLOTS] ;

void swap_init(){
  int i ;
  for(i =0 ; i < N_SWAP_SLOTS; ++i){
    swap_slots[i].is_free =1;
    swap_slots[i].page_perm=0;
    swap_slots[i].block_no=2 + (8*i);
  }
}

void swap_out(){
    // called the function find_victim , it handles to find the victim process and victim page . now the type of the victim page is pte* , but we need to convert it into char* so that we can write it on the disk
    // now we have victim process and victim page , but how do i get the address of this page in memory ?
    
    struct proc* victimproc ;
    victimproc = find_victim() ;
    if(victimproc==0){
        panic("No victim process found");
    }
    struct pde* pgdir = victimproc->pgdir ;  
    
    struct pte* victimpage = find_victim_page(pgdir,victimproc) ;
    if(victimpage==0){
        panic("No victim page found");
    }
    // we need to write the victim page on the disk , so that we can free up this memory .
    
    char* pg = (char*)P2V(PTE_ADDR(victimpage));  //convert pte* into char
  
    for(int i=0 ; i<N_SWAP_SLOTS;i++){
        if(swap_slots[i].is_free == 1){
            write_page_to_disk(1,pg,swap_slots[i].block_no);
            swap_slots[i].page_perm = PTE_FLAGS(victimpage) ;
        }
    }

}

void
map_address(pde_t *pgdir, uint addr)
{
	struct proc *curproc = myproc();
  pte_t *pte=walkpgdir(pgdir, (char*)addr, 0); // physical address of the page table entry 
	uint cursz= curproc->sz;
	uint a= PGROUNDDOWN(rcr2());			//rounds the address to a multiple of page size (PGSIZE)
  uint block = getswappedblk(pgdir,a);
	char *mem=kalloc() ;    //allocate a physical page // this is the virutal address of the memory that is allocated 
  read_page_from_disk(ROOTDEV, mem, block); // mem 
  *pte=V2P(mem) | PTE_W | PTE_U | PTE_P; // the flags need to be rethink
  lcr3(V2P(pgdir));
  bfree_page(ROOTDEV,block);
}
void
handle_pgfault()
{

	unsigned addr;
	struct proc *curproc = myproc();

	asm volatile ("movl %%cr2, %0 \n\t" : "=r" (addr));
	addr &= ~0xfff;
	map_address(curproc->pgdir, addr);
}


