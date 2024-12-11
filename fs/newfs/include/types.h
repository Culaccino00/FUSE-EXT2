#ifndef _TYPES_H_
#define _TYPES_H_

/******************************************************************************
* SECTION: Type def
*******************************************************************************/
typedef int          boolean;

typedef enum newfs_file_type {
    NFS_REG_FILE,
    NFS_DIR
} NFS_FILE_TYPE;
/******************************************************************************
* SECTION: Macro
*******************************************************************************/
#define MAX_NAME_LEN            128     
#define TRUE                    1
#define FALSE                   0
#define UINT32_BITS             32
#define UINT8_BITS              8

#define NFS_MAGIC_NUM           0x52415453  
#define NFS_SUPER_OFS           0
#define NFS_ROOT_INO            0



#define NFS_ERROR_NONE          0
#define NFS_ERROR_ACCESS        EACCES
#define NFS_ERROR_SEEK          ESPIPE     
#define NFS_ERROR_ISDIR         EISDIR
#define NFS_ERROR_NOSPACE       ENOSPC
#define NFS_ERROR_EXISTS        EEXIST
#define NFS_ERROR_NOTFOUND      ENOENT
#define NFS_ERROR_UNSUPPORTED   ENXIO
#define NFS_ERROR_IO            EIO     /* Error Input/Output */
#define NFS_ERROR_INVAL         EINVAL  /* Invalid Args */

#define NFS_MAX_FILE_NAME       128
#define NFS_INODE_PER_FILE      1
#define NFS_MAX_SIZE_PER_FILE   16
#define NFS_DEFAULT_PERM        0777

#define NFS_IOC_MAGIC           'S'
#define NFS_IOC_SEEK            _IO(SFS_IOC_MAGIC, 0)

#define NFS_FLAG_BUF_DIRTY      0x1
#define NFS_FLAG_BUF_OCCUPY     0x2
/******************************************************************************
* SECTION: Macro Function
*******************************************************************************/
#define NFS_LOGIC_SZ() (newfs_super.sz_logic)
#define NFS_IO_SZ() (newfs_super.sz_io)
#define NFS_DISK_SZ() (newfs_super.sz_disk)
#define NFS_DRIVER() (newfs_super.driver_fd)

#define NFS_ROUND_DOWN(value, round) ((value) % (round) == 0 ? (value) : ((value) / (round)) * (round))
#define NFS_ROUND_UP(value, round) ((value) % (round) == 0 ? (value) : ((value) / (round) + 1) * (round))

#define NFS_BLKS_SZ(blks) ((blks) * NFS_LOGIC_SZ())
#define NFS_ASSIGN_FNAME(pnewfs_dentry, _fname) memcpy((pnewfs_dentry)->name, (_fname), strlen((_fname)))
// data和inode的布局不一样，所以offset计算方式也不同
// 多个ino可以在同一个块内，一个dno代表一个块
#define NFS_INO_OFS(ino) (NFS_BLKS_SZ(newfs_super.ino_offset) + (ino) * sizeof(struct newfs_inode_d))//TODO: NFS_LOGIC_SZ
#define NFS_DATA_OFS(dno) (NFS_BLKS_SZ(newfs_super.data_offset + (dno)))

#define NFS_IS_DIR(pinode) ((pinode)->dentry->ftype == NFS_DIR)
#define NFS_IS_REG(pinode) ((pinode)->dentry->ftype == NFS_REG_FILE)
/******************************************************************************
* SECTION: FS Specific Structure - In memory structure
*******************************************************************************/
struct newfs_dentry;
struct newfs_inode;
struct newfs_super;

struct custom_options {
	char*        device;
    boolean            show_help;
};

struct newfs_inode
{
    // ino >= 0 && ino < ino_blks
    /* inode编号 */
    int                ino;                // 在inode位图中的下标
    /* TODO: Define yourself */

    /* 文件的属性 */
    int                size;               // 文件已占用空间
    NFS_FILE_TYPE      ftype;              // 文件类型（目录类型、普通文件类型）

    /* 数据块的索引 */
    int                block_pointer[NFS_MAX_SIZE_PER_FILE];   // 数据块指针（可固定分配）

    /* 其他字段 */
    int                 dir_cnt;            // 如果是目录类型文件，下面有几个目录项
    struct newfs_dentry *dentry;           /* 指向该inode的dentry */
    struct newfs_dentry *dentrys;          /* 所有目录项 */

    /* 文件 */
    uint8_t *data;
};


struct newfs_dentry
{
    /* 文件名称 */
    char name[NFS_MAX_FILE_NAME];
    /* inode编号 */
    uint32_t ino;
    /* TODO: Define yourself */

    /* 文件类型 */
    NFS_FILE_TYPE ftype;

    /* 多的字段 */
    struct newfs_dentry *parent;  /* 父亲Inode的dentry */
    struct newfs_dentry *brother; /* 兄弟Inode的dentry */
    struct newfs_inode *inode;    /* 文件对应的inode */
};

struct newfs_super
{
    uint32_t    magic_num;
    int         driver_fd;

    /* TODO: Define yourself */

    int         sz_io;
    int         sz_disk;
    int         sz_logic;
    int         sz_usage;

    /* 磁盘布局分区信息 */
    int         sb_offset;       // 超级块于磁盘中的偏移，通常默认为0
    int         sb_blks;         // 超级块于磁盘中的块数，通常默认为1
    int         ino_map_offset;  // 索引节点位图于磁盘中的偏移,1
    uint8_t *   map_inode;
    int         ino_map_blks;    // 索引节点位图于磁盘中的块数,1
    int         data_map_offset; // 数据块位图于磁盘中的偏移,2
    uint8_t *   map_data;
    int         data_map_blks;   // 数据块位图于磁盘中的块数,1
    int         ino_offset;      // 索引节点区于磁盘中的偏移,3
    int         ino_blks;        // 索引节点区于磁盘中的块数,= (max_file * sizeof(inode_d)) / sz_logic_blk
    int         data_offset;     // 数据块区于磁盘中的偏移
    int         data_blks;       // 数据块区于磁盘中的块数

    boolean     is_mounted;

    struct newfs_dentry* root_dentry;
};

static inline struct newfs_dentry* new_dentry(char *fname, NFS_FILE_TYPE ftype)
{
    struct newfs_dentry *dentry = (struct newfs_dentry *)malloc(sizeof(struct newfs_dentry));
    memset(dentry, 0, sizeof(struct newfs_dentry));
    NFS_ASSIGN_FNAME(dentry, fname);
    dentry->ftype   = ftype;
    dentry->ino     = -1;
    dentry->inode   = NULL;
    dentry->parent  = NULL;
    dentry->brother = NULL;
}
/******************************************************************************
 * SECTION: FS Specific Structure - Disk structure
 *******************************************************************************/
struct newfs_inode_d
{
    /* inode编号 */
    int                ino;                // 在inode位图中的下标

    /* 文件的属性 */
    int                size;               // 文件已占用空间
    NFS_FILE_TYPE      ftype;              // 文件类型（目录类型、普通文件类型）

    /* 数据块的索引 */
    int                block_pointer[NFS_MAX_SIZE_PER_FILE];   // 数据块指针（可固定分配）

    /* 其他字段 */
    int                dir_cnt;            // 如果是目录类型文件，下面有几个目录项
};

#define INODE_PER_BLK (NFS_LOGIC_SZ() / sizeof(struct newfs_inode_d))

struct newfs_dentry_d
{
    /* 文件名称 */
    char fname[NFS_MAX_FILE_NAME];
    /* inode编号 */
    uint32_t ino;
    /* 文件类型 */
    NFS_FILE_TYPE ftype;
};

#define DENTRY_PER_BLK (NFS_LOGIC_SZ() / sizeof(struct newfs_dentry_d))

struct newfs_super_d
{
    uint32_t magic_num;
    int sz_usage;

    /* 磁盘布局分区信息 */
    int sb_offset;       // 超级块于磁盘中的偏移，通常默认为0
    int sb_blks;         // 超级块于磁盘中的块数，通常默认为1
    int ino_map_offset;  // 索引节点位图于磁盘中的偏移,1
    int ino_map_blks;    // 索引节点位图于磁盘中的块数,1
    int data_map_offset; // 数据块位图于磁盘中的偏移,2
    int data_map_blks;   // 数据块位图于磁盘中的块数,1
    int ino_offset;      // 索引节点区于磁盘中的偏移,3
    int ino_blks;        // 索引节点区于磁盘中的块数,= (max_file * sizeof(inode_d)) / sz_logic_blk
    int data_offset;     // 数据块区于磁盘中的偏移
    int data_blks;       // 数据块区于磁盘中的块数
};

#endif /* _TYPES_H_ */