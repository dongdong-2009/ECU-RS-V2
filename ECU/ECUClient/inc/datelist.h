#ifndef _DATELIST_H_
#define _DATELIST_H_

typedef struct list
{
    int date;		//时间点
    struct list *next;	//下一个结点
    struct list *prev;
}LDateNode,*LinkDateList;

//创建一个链表头
struct list *list_create(int date);

//把一个数据插入到链表头的后面
int list_addhead(struct list *head, int date);

//把一个数据插入到链表尾的后面
int list_addtail(struct list *head, int date);

//打印链表
void list_print(struct list *head);

//销毁链表
void list_destroy(struct list **entry);
//链表的排序
void list_sort(struct list *head);

#endif
