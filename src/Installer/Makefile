all:	../fdimage.img
	cd ../tools && $(MAKE)
	cd ../libc && $(MAKE)
	cd ../MicroKernel && $(MAKE)
	cd ../Kernel && $(MAKE)
	cd ../MyLoader && $(MAKE)
	
	
	../tools/ramdiskCreator.x ../Kernel/rd.img ../Kernel
	
	cp ../MyLoader/*.img ../
	cp ../MicroKernel/*.img ../
	cp ../Kernel/*.img ../
	
	#cp ../NoSe.flp ../fdimage.img
	 
	#/sbin/losetup /dev/loop0 ../fdimage.img
	#mount /dev/loop0 -o loop /mnt/fda
	#chmod 777 /mnt/fda

	#mv ../MicroKernel/*.img /mnt/fda
	#mv ../Kernel/*.img /mnt/fda
	
	#umount /mnt/fda
	#sleep 0.1	
	#losetup -d /dev/loop0
	
	#dd if=../fdimage.img of=/dev/fd0 bs=512 count=2880

	
run: all
	#qemu -m 128 -boot order=a -fda ../fdimage.img -drive file="../disco.img",index=0,media=disk,if=ide
	# -curses
	qemu -m 128 -kernel ../myLoader.img -initrd "../microkernel.img,../OS.img,../rd.img" -drive file="../disco.img",index=0,media=disk,if=ide
clean:
	cd ../MicroKernel && $(MAKE) clean
	cd ../Kernel && $(MAKE) clean
	
	
