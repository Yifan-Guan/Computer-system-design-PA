#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;
  size_t fd = fs_open(filename, 0, 0);
  char *load_va, *load_pg=0;
  uintptr_t pa_start;

  Log("Loading program '%s' from ramdisk, fd = %d", filename, fd);

  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(ehdr.e_ident[0] == 0x7f && ehdr.e_ident[1] == 'E' && ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F');

  for (int i = 0; i < ehdr.e_phnum; i++) {
    fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
    fs_read(fd, &phdr, sizeof(Elf_Phdr));

    if (phdr.p_type == PT_LOAD) {
      load_va = (char*) ( phdr.p_vaddr & 0xfffff000);
      pa_start = 0;

      fs_lseek(fd, phdr.p_offset, SEEK_SET);

      if (pcb) {
        while ((uintptr_t)load_va <= phdr.p_vaddr + phdr.p_memsz) {
          load_pg = new_page(1);
          assert(&(pcb->as));
          map(&(pcb->as), load_va, load_pg, 0b111);

          fs_read(fd, load_pg + (phdr.p_vaddr&0xfff), PGSIZE);

          load_va += PGSIZE;
          if (!pa_start) pa_start = (uintptr_t)load_pg;
        }

        memset((char*)(pa_start+(phdr.p_vaddr&0xfff)+phdr.p_filesz), 0, phdr.p_memsz-phdr.p_filesz);
        pcb->max_brk = (uintptr_t)load_va;

      } else {
        fs_read(fd, load_va, phdr.p_filesz);
        memset((char*)(phdr.p_vaddr+phdr.p_filesz), 0, phdr.p_memsz-phdr.p_filesz);
      }
    }
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

