#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <arch/i386/idt.h>
#include <arch/i386/pic.h>

namespace {
  void fillDesc(uint32_t offset, uint16_t selector, uint8_t type, IdtDesc *desc) {
    desc->offset_low  =  offset & 0x0000FFFF;
    desc->offset_high = (offset & 0xFFFF0000) >> 16;
    
    desc->selector = selector;
    desc->zero = 0;
    desc->type = type;
  }
}

IdtDesc idt[IDT_SIZE];
IdtReg idtr;

// Wrappers are defined in isr.s and will call methods here.
extern "C" {
  void asmIrqDummy();
  void irqDummy() {
    Pic::sendEoi();
  }

  void asmIrqTick();
  void irqTick() {
    /*
    static uint32_t ticks = 0;
    ticks++;
    printf("ticks: %u\n", ticks);
    */
    Pic::sendEoi();
  }
}

void Idt::init() {
  // Interrupt vectors:
  //   0x00 -> 0x1F        are for CPU exceptions.
  //   0x20 -> 0x27 (32->) are for master interrupts.
  //   0x28 -> 0x2F (40->) are for slave interrupts.
  // (Defined pic.cpp)

  // Initialize IRQ (interrupt requests) with dummy routines because
  // the entries must be defined.
  for (size_t i = 0; i < IDT_SIZE; i++) {
    fillDesc((uint32_t) asmIrqDummy, IRQ_TIMER, INTR_GATE, &idt[i]);
  }

  // Master interrupt 0: Hardware timer tick
  fillDesc((uint32_t) asmIrqTick, IRQ_TIMER, INTR_GATE, &idt[32]);

  // Create idt register and put it at the base memory address.
  idtr.limit = IDT_SIZE * sizeof(IdtDesc);
  idtr.base = IDT_BASE;
  memcpy((void*) idtr.base, (void*) idt, idtr.limit);

  // Load idt.
  __asm__("lidtl (idtr)");
}
