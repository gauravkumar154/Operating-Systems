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
    pde_t* pgdir = victimproc->pgdir ;  
    pte_t* victimpage = find_victim_page(pgdir,victimproc) ;
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
  uint flag = PTE_FLAGS(*pte);
	uint cursz= curproc->sz;
  uint block = getswappedblk(pgdir,addr);
	char *mem=kalloc() ;    //allocate a physical page // this is the virutal address of the memory that is allocated 
  read_page_from_disk(ROOTDEV, mem, block); // mem 
  *pte=V2P(mem) | flag | PTE_P ; // the flags need to be rethink
  lcr3(V2P(pgdir));
}
void
handle_pgfault()
{
	struct proc *curproc = myproc();

	uint a= PGROUNDDOWN(rcr2());
	map_address(curproc->pgdir, a);
}


