#ifndef _DATELIST_H_
#define _DATELIST_H_

typedef struct list
{
    int date;		//ʱ���
    struct list *next;	//��һ�����
    struct list *prev;
}LDateNode,*LinkDateList;

//����һ������ͷ
struct list *list_create(int date);

//��һ�����ݲ��뵽����ͷ�ĺ���
int list_addhead(struct list *head, int date);

//��һ�����ݲ��뵽����β�ĺ���
int list_addtail(struct list *head, int date);

//��ӡ����
void list_print(struct list *head);

//��������
void list_destroy(struct list **entry);
//���������
void list_sort(struct list *head);

#endif
