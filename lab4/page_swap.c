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

char* swap_out(){
    // called the function find_victim , it handles to find the victim process and victim page . now the type of the victim page is pte* , but we need to convert it into char* so that we can write it on the disk
    // now we have victim process and victim page , but how do i get the address of this page in memory ?
    
    // struct proc* victimproc ;
    // victimproc = find_victim() ;
    // // cprintf("swap_out: %d\n",victimproc->pid);
    // victimproc->rss--;
    // if(victimproc==0){
    //     panic("No victim process found");
    // }
    // pde_t* pgdir = victimproc->pgdir ;  
    // pte_t victimpage = find_victim_page(pgdir,victimproc) ;
    // if(victimpage==0){
    //     panic("No victim page found");
    // }
    // // we need to write the victim page on the disk , so that we can free up this memory .
    
    // char* pg = (char*)P2V(PTE_ADDR(victimpage));  //convert pte* into char
    
    for(int i=0 ; i<N_SWAP_SLOTS;i++){
        if(swap_slots[i].is_free == 1){
            struct proc* victimproc ;
            victimproc = find_victim() ;
            // cprintf("swap_out: %d\n",victimproc->pid);
            victimproc->rss--;
            pde_t* pgdir = victimproc->pgdir ;  
            pte_t victimpage = find_victim_page(pgdir,victimproc , i) ;           
            char* pg = (char*)P2V(PTE_ADDR(victimpage));
            cprintf("swap_out: %d , rss : %d\n",victimproc->pid , victimproc->rss);
            write_page_to_disk(pg,swap_slots[i].block_no);
            cprintf("block written to disk\n");
            swap_slots[i].page_perm = PTE_FLAGS(victimpage) ;
            swap_slots[i].is_free = 0;
            memset(pg,0,PGSIZE);
            cprintf("swap_out done\n");
            return pg;
        }
    }
    
    panic("No free swap slot found");
    return 0;
       

}

// void
// map_address(pde_t *pgdir, uint addr)
// {
//   pte_t *pte=walkpgdir(pgdir, (char*)addr, 0); // physical address of the page table entry 
//   uint flag = PTE_FLAGS(*pte);
//   uint block = getswappedblk(pgdir,addr);
// 	char *mem=kalloc() ;    //allocate a physical page // this is the virutal address of the memory that is allocated 
//   read_page_from_disk(ROOTDEV, mem, block); // mem 
//   *pte=V2P(mem) | flag | PTE_P ; // the flags need to be rethink
//   lcr3(V2P(pgdir));
// }
void
handle_pgfault()
{
  cprintf("here in handle_pgfault\n");
	struct proc *curproc = myproc();
	uint addr = PGROUNDDOWN(rcr2());
  pde_t *pgdir = curproc->pgdir;
	pte_t *pte = walkpgdir_copy(pgdir, (char*)addr, 0); // physical address of the page table entry 
  int swap_slot_no = PTE_ADDR(*pte) >> 12;
  struct swap_slot swap_slot = swap_slots[swap_slot_no];
  uint block = swap_slot.block_no;
  uint perms = swap_slot.page_perm;
	char *mem=kalloc() ;    //allocate a physical page // this is the virutal address of the memory that is allocated 
  read_page_from_disk(mem, block); 
  *pte=V2P(mem) | perms | PTE_P ; 
  swap_slot.is_free = 1; 
  curproc->rss++;
  cprintf("handle_pgfault done\n");
}


