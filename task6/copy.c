#include <stdio.h>				// Standered I/O for consol printf()
//#include <sys/types.h> 		// used for data types used in the code like size_t
#include <sys/stat.h>		// to get the status of the file using fstat()
#include <fcntl.h> 				// to perform/manipulate on files given with file descriptors
#include <unistd.h>			// Used to call OS related functions (getopt function)
#include <string.h> 			// for  memcpy and for size_t type
#include <sys/mman.h>	// used for mmap and munmap

//#define BUFSIZE 4096		// the buffer size for the file.
int buf_size;             //got the buffer size dynamically (using fstat.size_t) instead of the line above
int copy_read_write(int fd_from,int fd_to);
int copy_mmap(int fd_from, int fd_to);

int main (int argc, char *argv[])
{
	struct stat st;											// structure for the fstat function to handle size
	int fd_from = 0, fd_to = 0;							// File descriptor variables
	char c, with_mmap = 0, disp_msg = 0;		//characters for the getopt and to determine the input choice
	
	
	//--------------------------PART ONE------------------------------------
	// getopt(count, vector, "options to be parsed") returns the char in options and -1 when there are no more options
	// The switch case checks the return value and adjust variables acordingly for part two of the algorithm.	
	while((c = getopt (argc, argv, "mh")) != -1){ 	
		switch (c){
			case 'm':
				with_mmap = 1;
				break;
			case 'h':
				disp_msg = 1;
				break;
			default:
				printf("\nERROR: Unknown option character.\n");
				disp_msg = 1;
		}
	}
	
	
	//---------------------------PART TWO------------------------------------
	// Check if the -h option was given as input to display the message (help) else open/create files.
	
	
	// open() function returns the file descriptor that will be used later or returns -1 in case of errors
	// its parameters are ([file name], flags) two type of flags, file creation and access mode
	// Flags used:-
	// O_RDONLY = request reading the file in read only mode.
	// O_CREAT = create file if it doesn't exist
	// O_RDWR = request reading the file in read and write mode
	// O_TRUNC = Truncate the previouse content of the file (if iit exists)
	// access(file path, mode) the F_OK mode means file exists
	// optind is for the extra arguments which are not parsed (in our case will be the source and destination files)
	if (!disp_msg){                                               						  // If the 'h'character is not entered by user
		if(access( argv[optind], F_OK ) != -1 ) {
			if ((fd_from = open(argv[optind], O_RDONLY)) < 0){           // If source file for the given filename cannot be opened in READ ONLY MODE
				printf("\nERROR: Can't open file for reading.\n");     		  // Display error message 
				return 1;
			}
		}
		else {
			printf("\nERROR: source file does not exist\n");
			return 1;
		}
		// Create destination file, If destination file cannot be opened for writing, created or truncated then display error message
		if ((fd_to = open(argv[optind+1], O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0){  		
			printf("\nERROR: Can't open or create file for writing.\n");                      
			return 1;
		}
		
		if (with_mmap){                                      //If mmap=1 ('m' option is entered by user), copy with mmap functtion
			printf("\nCopying '%s' into '%s' using mmap() and memcpy() functions.\n\n",argv[optind],argv[optind+1]);
			copy_mmap(fd_from, fd_to);
		}
		
		// If no character is specified, call copy_read_write function
		else{              
			if(fstat(fd_from, &st) < 0)
				printf("\nERROR: Failed to get status of the source file!\n");
			buf_size = st.st_size;
			//printf("%i\n", buf_size);
			printf("\nCopying '%s' into '%s' using read() and write() functions.\n\n",argv[optind],argv[optind+1]);
			copy_read_write(fd_from, fd_to);
		}
		
		// close(file descriptor)
		// close() functionreturns -1 in case of errors
		 // Closing both files or displaying error message
		if (close(fd_from))
			printf("\nERROR: Can't close %s!\n", argv[optind]);
		if (close(fd_to))                                             
			printf("\nERROR: Can't close %s!\n", argv[optind]);
	} 
	
	// disp_msg value is 1 to display help message.
	else
		printf("This program copies one file to another. Syntax:\n\tcopy [-m]<file_name> <new_file_name>\n\tcopy [-h]\nUse -m option for mmap and memcpy\nLeave blank for read() and write() option.\n\n");
	return 0;
}


//----------------------------PART THREE----------------------------------
// The functions that will copy the content based on the user input

int copy_read_write(int fd_from, int fd_to){
	
	char buf[buf_size];          		 // Get buffer size
	size_t cpy_bytes;            		// Get return values of the read and write functions
	size_t nbyte = sizeof(buf);  	// Define number of bytes to be copied
	
	// both read and write functions return -1 on error
	// read(fd source, the buffer to read to, number of bytes to read)
	cpy_bytes = read(fd_from, &buf, nbyte);	
	// If read function returned -1 print error message
	if (cpy_bytes < 0)                                  
		printf("\nERROR: Error reading input file!\n");	
	
	// write(fd of destination, buffer to write, number of bytes) 
	cpy_bytes = write(fd_to, &buf, cpy_bytes);
	if (cpy_bytes < 0) 
		printf("\nERROR: Error writing into output file!\n");
}

int copy_mmap(int fd_from, int fd_to){
	struct stat fileStat;
	void *src, *dst;                  //Define pointers to source and destination files for mmap & memcpy  
	if(fstat(fd_from,&fileStat) < 0){
		printf("\nERROR: Failed to get status of the source file!\n");
		return 1;
	}
	// truncate destination file to a specified length (length of fileStat "source")
	// ftruncate(file descriptor, size length)
	ftruncate(fd_to,fileStat.st_size);
	// mmap(starting_address, length, mode, flags, file descriptor, offset)
	// MAP_SHARED = this mapping is shared with other processes.
	// On errors, mmap() returns MAP_FAILED
	if ((src = mmap(0, fileStat.st_size, PROT_READ, MAP_SHARED, fd_from, 0)) == MAP_FAILED){       	
		 printf("\nERROR: mmap for the source file failed!\n");  }                      // If shared mapping of src with read permission fails, display appropriate error message
	
	if ((dst = mmap(0, fileStat.st_size, PROT_WRITE, MAP_SHARED, fd_to, 0)) == MAP_FAILED){        
		 printf("\nERROR: mmap for the output file failed!\n");   }                      // If shared mapping of dst with write permission fails, display appropriate error message
	
	// memcpy(source pointer, destination pointer, size_of_source to be copied)
	memcpy(dst, src, fileStat.st_size);
	
	// munmap(pointer to content to be deleted from mapping, length)
	// returns -1 on errors
	if (munmap(src, fileStat.st_size) < 0)                                                      
		 printf("\nERROR: munmap for the input file failed!\n"); 
	if (munmap(dst, fileStat.st_size) < 0)                                                 
		 printf("\nERROR: munmap for the output file failed!\n"); 

}
