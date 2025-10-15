Virtual Address 6c74:  --> pde index:0x1b  pde contents:(valid 1, pfn 0x20)    --> pte index:0x3  pte contents:(valid 1, pfn 0x61)      --> Translates to Physical Address 0xc34 --> Value: 6

Virtual Address 6b22:  --> pde index:0x1a  pde contents:(valid 1, pfn 0x52)    --> pte index:0x19  pte contents:(valid 1, pfn 0x47)      --> Translates to Physical Address 0x8e2 --> Value: 26

Virtual Address 03df:  --> pde index:0x0  pde contents:(valid 1, pfn 0x5a)    --> pte index:0x1e  pte contents:(valid 1, pfn 0x5)      --> Translates to Physical Address 0xbf --> Value: 15

Virtual Address 69dc:  --> pde index:0x1a  pde contents:(valid 1, pfn 0x52)    --> pte index:0xe  pte contents:(valid 0, pfn 0x7f)      --> Fault (page table entry not valid)

Virtual Address 317a:  --> pde index:0xc  pde contents:(valid 1, pfn 0x18)    --> pte index:0xb  pte contents:(valid 1, pfn 0x35)      --> Translates to Physical Address 0x6ba --> Value: 30

Virtual Address 4546:  --> pde index:0x11  pde contents:(valid 1, pfn 0x21)    --> pte index:0xa  pte contents:(valid 0, pfn 0x7f)      --> Fault (page table entry not valid)

Virtual Address 2c03:  --> pde index:0xb  pde contents:(valid 1, pfn 0x44)    --> pte index:0x0  pte contents:(valid 1, pfn 0x57)      --> Translates to Physical Address 0xae3 --> Value: 22

Virtual Address 7fd7:  --> pde index:0x1f  pde contents:(valid 1, pfn 0x12)    --> pte index:0x1e  pte contents:(valid 0, pfn 0x7f)      --> Fault (page table entry not valid)

Virtual Address 390e:  --> pde index:0xe  pde contents:(valid 0, pfn 0x7f)      --> Fault (page directory entry not valid)

Virtual Address 748b:  --> pde index:0x1d  pde contents:(valid 1, pfn 0x0)    --> pte index:0x4  pte contents:(valid 0, pfn 0x7f)      --> Fault (page table entry not valid)

实现逻辑见`impl.ipynb`文件。