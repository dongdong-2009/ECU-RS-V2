#include "datelist.h"
#include "stdio.h"
#include "stdlib.h"
#include <rtthread.h>
//创建一个链表头
struct list *list_create(int date)
{
    struct list *head = NULL;

    head = malloc(sizeof(struct list));
    if(head == NULL)
    {
        printf("malloc head error!!!\n");
        return NULL;
    }

    head->date = date;
    head->next = head->prev = head;

    return head;
}

//把new这个结点连接到prev与next之间
static void _insert(struct list *prev, struct list *next, struct list *new)
{
    prev->next = new;
    next->prev = new;
    new->prev = prev;
    new->next = next;
}

//把一个数据插入到链表头的后面
int list_addhead(struct list *head, int date)
{
    struct list *new = NULL;

    //为结点分配空间
    new = malloc(sizeof(struct list));
    if(new == NULL)
    {
        printf("malloc new error!!!\n");
        return -1;
    }

    //将数据保存到结点中
    new->date = date;

    //把结点连接到链表头的后面
    _insert(head, head->next, new);

    return 0;
}


//把一个数据插入到链表尾的后面
int list_addtail(struct list *head, int date)
{
    struct list *new = NULL;

    //为结点分配空间
    new = malloc(sizeof(struct list));
    if(new == NULL)
    {
        printf("malloc new error!!!\n");
        return -1;
    }

    //为结点存放数据分配空间
    new->date = date;

    //把结点连接到链表尾的后面
    _insert(head->prev, head, new);

    return 0;
}



//打印链表
void list_print(struct list *head)
{
    struct list *tmp;

    for(tmp = head->next; tmp != head; tmp = tmp->next)
    {
        printf("%d\n",tmp->date);
    }

    printf("\n");
}

//销毁链表
void list_destroy(struct list **entry)
{
    struct list *head, *tmp, *next;

    if((entry == NULL) || (*entry == NULL))
        return;

    head = *entry;
    for(tmp = head->next; tmp != head; tmp = next)
    {
        next = tmp->next;
        free(tmp);
    }
    free(head);

    *entry = NULL;
}

void list_sort(struct list *head)
{
    struct list *tmp, *new, *cur;

    if((head == NULL) || (head->next == head) || (head->next->next == head))
    {
        return;
    }

    //创建一个新链表头
    new = list_create(-1);

    //从head里面取出第一个结点放到new链表里面去
    cur = head->next;
    head->next = cur->next;
    cur->next->prev = head;
    _insert(new, new->next, cur);


    while(head->next != head)
    {
        cur = head->next;
        head->next = cur->next;
        cur->next->prev = head;

        for(tmp = new->next; tmp != new; tmp = tmp->next)
        {
            if(cur->date >= tmp->date)
            {
                _insert(tmp->prev, tmp, cur);
                break;
            }
        }

        if(tmp == new)
        {
            _insert(new->prev, new, cur);
        }
    }

    //更换链表头
    head->next = new->next;
    new->next->prev = head;
    new->prev->next = head;
    head->prev = new->prev;
    free(new);
    new = NULL;


}

#ifdef RT_USING_FINSH
#include <finsh.h>
void listt(void)
{
    int date[10] = {20180401,20180322,20180325,20180321,20180314,20180417,20180413,20180414,20180412,20180416};
    int i = 0;
    struct list *Head;
    Head = list_create(-1);

    for(i=0;i<10;i++){
        list_addhead(Head,date[i]);
    }

    list_print(Head);

    printf("list_sort--------->\n");
    list_sort(Head);

    list_print(Head);
    list_destroy(&Head);

}
FINSH_FUNCTION_EXPORT(listt, eg:listt());
#endif
