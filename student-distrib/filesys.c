#include "filesys.h"
#include "system_call.h"

static uint8_t* dataBlock_start; /* Pointer to data block */
static bootBlock_t* boot_block_ptr; /* Pointer to boot block */


/** 
 * init_filesys
 * 
 * Description: Initialize file system: boot block and data block based on mod address
 * 
 * Inputs: mod - file system module from kernel.c which contains file system start
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Populates file scope variables
 */
int32_t init_filesys(uint32_t* mod){
    
    if (mod == NULL) return -1;

    boot_block_ptr = (bootBlock_t*)mod;
    dataBlock_start = (uint8_t*)(boot_block_ptr + 1 + boot_block_ptr->n);
    // cur_open_file = NULL;

    return 0;
}


/** 
 * read_dentry_by_name
 * 
 * Description: Finds the dentry with name fname and fills dentry pointer with its information
 * 
 * Inputs: fname - name of file to find
 *         dentry - pointer to dentry to fill
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Fills dentry pointer
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    int i;

    if (fname == NULL || dentry == NULL){ return -1;}
    if (strlen((int8_t*)fname) == 0 || strlen((int8_t*)fname) > MAX_FILE_NAME_LEN)
        return -1;
        
    int8_t filename[MAX_FILE_NAME_LEN + 1];
    strncpy((int8_t *) filename, (int8_t *) fname, MAX_FILE_NAME_LEN);
    filename[MAX_FILE_NAME_LEN + 1] = 0x0;

    // find dentry in sysmem by comparing filename
    for (i=0; i < NUM_DENTRIES; i++) {
        if (strncmp(boot_block_ptr->dentries[i].fileName, filename, MAX_FILE_NAME_LEN) == 0) {
            //actually fill the struct
            strcpy(dentry->fileName , filename);

            // fileType is 32 bytes into dentry struct
            dentry->fileType = *((uint8_t*)&boot_block_ptr->dentries[i] + 32);
            // inode number is 36 bytes into dentry struct
            dentry->inodeNum = *((uint8_t*)&boot_block_ptr->dentries[i] + 36);

            return 0;
        }
    }
    // fname not found
    return -1;
}

/** 
 * read_dentry_by_index
 * 
 * Description: Finds the dentry with given index and fills dentry pointer with its information
 * 
 * Inputs: index - index of file to find
 *         dentry - pointer to dentry to fill
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Fills dentry pointer
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    // fill in dentry struct with info from index
    if (dentry == NULL || index < 0 || index >= MAX_DENTRIES){ return -1;}

    memcpy(dentry->fileName, &boot_block_ptr->dentries[index], MAX_FILE_NAME_LEN);

    // fileType is 32 bytes into dentry struct
    dentry->fileType = *((uint8_t*)&boot_block_ptr->dentries[index] + 32);
    // inode number is 36 bytes into dentry struct
    dentry->inodeNum = *((uint8_t*)&boot_block_ptr->dentries[index] + 36);

    return 0;
}


/** 
 * read_data
 * 
 * Description: Fills the given buffer with "length" bytes from file pointed to from "inode" starting at "offset" bytes
 * 
 * Inputs: inode - inode number
 *         offset - offset in file to begin reading from
 *         buf - buffer to read data into
 *         length - length of bytes to read
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Fills dentry pointer
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    // copy char data from filesys into the buf
    
    // can't check that inode actually corresponds to file
    if ( inode < 0 || inode > MAX_DENTRIES || offset < 0 || buf == NULL ) {
        return -1;
    }

    //init some offsets, position vars
    int i;
    int tmp_offset = offset;
    int32_t bytes_read = 0;
    uint32_t position;
    inode_t* cur_node = (inode_t*) (boot_block_ptr + inode + 1); // start of filesys + inode# + 1 for the bootblock
    // uint32_t fileLength = ((inode_t*)(boot_block_ptr + inode + 1))->length;

    uint32_t blockIdx = 0;
    uint32_t curDataBlockIdx = cur_node->dataBlocks[blockIdx];

    // if offset >= Block size need to start at the proper Block, used for multiple reads of same file
    if (tmp_offset >= BLOCK_SIZE) {
        curDataBlockIdx = cur_node->dataBlocks[tmp_offset / BLOCK_SIZE];
        tmp_offset = tmp_offset % BLOCK_SIZE;
    }

    // looping through length bytes of the file
    for (i = 0, position = 0; i < length; i++, position++) {
        if (bytes_read >= cur_node->length || (offset + position) >= cur_node->length) {
            return bytes_read;
        }

        // update block if weve reached the end of one
        if (position + tmp_offset >= BLOCK_SIZE) {
            blockIdx++;
            curDataBlockIdx = cur_node->dataBlocks[blockIdx];
            tmp_offset = 0;
            position = 0;

            // if bad data block number is found within file bounds of inode, return -1
            // check for bad block number
            if (curDataBlockIdx == NULL || blockIdx > MAX_DATA_BLOCKS) {
                return -1;
            }
        }

        // copy a byte from the file into the buffer
        buf[i] = *((uint8_t*) (dataBlock_start + curDataBlockIdx*BLOCK_SIZE + tmp_offset + position));
        bytes_read++;
    }

    return bytes_read;
}

/** 
 * getFileLen
 * 
 * Description: Returns file length of open file
 * 
 * Inputs: None
 * 
 * Outputs: Length of open file in bytes
 * 
 * Side Effects: None
 */
int32_t getFileLen(int32_t fd){
    pcb_t* pcb = getPCB();

    return ((inode_t*)(boot_block_ptr + pcb->fda[fd].inode + 1))->length;
}

/** 
 * get_file_len_by_dentry
 * 
 * Description: Returns file length of file pointed to by dentry
 * 
 * Inputs: None
 * 
 * Outputs: Length of dentry file in bytes
 * 
 * Side Effects: None
 */
int32_t get_file_len_by_dentry(dentry_t dentry){
    return ((inode_t*)(boot_block_ptr + dentry.inodeNum + 1))->length;
}

/** 
 * file_open
 * 
 * Description: opens file with given filename; return 0 on success, -1 on failure
 * 
 * Inputs: fname - name of file to open
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Calls read_dentry_by_name
 *               resets offset to 0
 *               allocates new dentry struct
 */
int32_t file_open (int32_t fd, const uint8_t* filename) {

	pcb_t* pcb = getPCB();			// calculate current process pcb

	dentry_t file;
	if (read_dentry_by_name(filename, &file))				// check if file exists
		return -1;

    pcb->fda[fd].fops = (uint32_t *) file_fops;             // open file and place into fda array
    pcb->fda[fd].inode = file.inodeNum;
    pcb->fda[fd].file_position = 0;
    pcb->fda[fd].flags = 1;

    return 0;                                               // return success

}

/** 
 * file_close
 * 
 * Description: closes file; return 0 on success, -1 on failure
 * 
 * Inputs: fd - file to close
 * 
 * Outputs: 0 on success
 * 
 * Side Effects: Sets cur_open_file to NULL
 */
int32_t file_close (int32_t fd) {
    // undo open function
    return 0;
}

/** 
 * file_read
 * 
 * Description: Reads nbytes from fd into buf
 * 
 * Inputs: fd - file to read
 *         buf - buffer to read data into
 *         nbytes - number of bytes to read
 * 
 * Outputs: Number of bytes read on success, -1 on failure
 * 
 * Side Effects: Calls read_data
 *               Increments offset by number of bytes read
 */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes) {
    // for now ignore fd, use cur_open_file

    if (buf == NULL || nbytes < 0) return -1;

    pcb_t* pcb = getPCB();

    if (getFileLen(fd) < nbytes) {
        nbytes = getFileLen(fd);
    }

    int32_t bytes_read = read_data (pcb->fda[fd].inode, pcb->fda[fd].file_position, buf, nbytes);

    if (bytes_read != -1) {
        pcb->fda[fd].file_position += bytes_read;
    }

    return bytes_read;
}

/** 
 * file_write
 * 
 * Description: Currently does nothing
 * 
 * Inputs: fd - file to write
 *         buf - buffer to write data into
 *         nbytes - number of bytes to write
 * 
 * Outputs: -1
 * 
 * Side Effects: None
 */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/** 
 * dir_open
 * 
 * Description: opens directory with given dirname
 * 
 * Inputs: dirname - name of directory to open
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Calls read_dentry_by_name
 *               allocates new dentry struct
 */
int32_t dir_open (int fd, const uint8_t* dirname) {
    if (!dirname)
        return -1;

	pcb_t* pcb = getPCB();			// calculate current process pcb

    pcb->fda[fd].fops = (uint32_t *) directory_fops;        // setting up directory entry in fda
    pcb->fda[fd].inode = -1;
    pcb->fda[fd].file_position = 0;
    pcb->fda[fd].flags = 1;

    return 0;                                               // return success

}

/** 
 * dir_close
 * 
 * Description: closes directory
 * 
 * Inputs: fd - dir to close
 * 
 * Outputs: 0 on success
 * 
 * Side Effects: None
 */
int32_t dir_close (int32_t fd) {
    // probably does nothing, return 0
    return 0;
}


/** 
 * dir_read
 * 
 * Description: Reads a filename from directory into buf
 * 
 * Inputs: fd - file to read
 *         buf - buffer to read data into
 * 
 * Outputs: 0 on success, -1 on failure
 * 
 * Side Effects: Calls read_dentry_by_index
 *               Fills buffer with file name
 */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes) {

	pcb_t* pcb = getPCB();			// calculate current process pcb

    int fileNameBytesRead;

    if (pcb->fda[fd].file_position >= boot_block_ptr->numDentries) return 0;

    char* cbuf = (char*)buf;
    dentry_t curDentry;
    read_dentry_by_index(pcb->fda[fd].file_position, &curDentry);

    uint32_t len = strlen(curDentry.fileName);
    if (len > MAX_FILE_NAME_LEN) {len = MAX_FILE_NAME_LEN;}

    // uint32_t idx = len;
    strncpy(cbuf, curDentry.fileName, len); // copy file name to the buffer
    // cbuf[idx] = ' ';
    // idx++;
    fileNameBytesRead= len; // need to return Bytes read for the sysCalls to act properly

    // for prior checkpoint we were adding the file type and file size, doesn't want it here

    // We want the decimal number, so pass in 10 to itoa
    // int8_t* str = itoa(curDentry.fileType, cbuf+idx, 10);
    // idx += strlen(str);
    // cbuf[idx] = ' ';
    // idx++;

    // if (curDentry.fileType == FILE) {
    //     uint32_t fileSize = ((inode_t*)(boot_block_ptr + curDentry.inodeNum + 1))->length;
    //     // We want the decimal number, so pass in 10 to itoa
    //     itoa(fileSize, cbuf+idx, 10);
    // }
    pcb->fda[fd].file_position++;   // only one filename is read to the buffer each call of dir_read
    return fileNameBytesRead;
}

/** 
 * dir_write
 * 
 * Description: Currently does nothing
 * 
 * Inputs: fd - dir to write
 *         buf - buffer to write data into
 *         nbytes - number of bytes to write
 * 
 * Outputs: -1
 * 
 * Side Effects: None
 */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
