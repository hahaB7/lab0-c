#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        q_release_element(node);
    }
    free(head);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;
    int s_len = strlen(s);
    element->value = (char *) malloc((s_len + 1) * sizeof(char));
    if (!element->value) {
        free(element);
        return false;
    }
    strncpy(element->value, s, (s_len + 1));
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s) {
        return false;
    }
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *tmp = list_entry(head->next, element_t, list);
    if (sp && bufsize > 0) {
        strncpy(sp, tmp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&tmp->list);
    return tmp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *tmp;

    list_for_each (tmp, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_del(slow);
    element_t *element = list_entry(slow, element_t, list);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return false;
    element_t *node = NULL, *safe = NULL, *dup_tail = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        if (&safe->list != head && !strcmp(node->value, safe->value)) {
            list_del(&node->list);
            q_release_element(node);
            dup_tail = safe;
        } else {
            if (dup_tail) {
                list_del(&dup_tail->list);
                q_release_element(dup_tail);
            }
            dup_tail = NULL;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *first = head->next, *second = head->next->next;
    while (second != head && first != head) {
        list_move(second, first->prev);
        first = first->next;
        second = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node = NULL, *safe = NULL;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head) || k == 1)
        return;
    int len = q_size(head), cnt = 0;
    struct list_head *node = NULL, *safe = NULL, *reverse_head = head;

    if (len < k)
        return;
    list_for_each_safe (node, safe, head) {
        list_move(node, reverse_head);
        if (++cnt == k) {
            if ((len -= k) < k)
                return;
            cnt = 0;
            reverse_head = safe->prev;
        }
    }
}
struct list_head *merge(struct list_head *left, struct list_head *right)
{
    LIST_HEAD(dummy_head);
    struct list_head *list = &dummy_head;
    while (left && right) {
        const char *s1 = list_entry(left, element_t, list)->value,
                   *s2 = list_entry(right, element_t, list)->value;
        if (strcmp(s1, s2) <= 0) {
            list->next = left;
            left = left->next;
        } else {
            list->next = right;
            right = right->next;
        }
        list = list->next;
    }
    list->next = left ? left : right;
    return dummy_head.next;
}

struct list_head *mergeSort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *slow = head, *fast = head->next, *right = NULL;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    right = slow->next;
    slow->next = NULL;
    head = mergeSort(head);
    right = mergeSort(right);
    return merge(head, right);
}
/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *data_head = head->next, *node = NULL, *safe = NULL;
    head->prev->next = NULL;
    head->next = mergeSort(data_head);

    for (node = head, safe = head->next; safe->next;
         node = safe, safe = node->next) {
        safe->prev = node;
    }
    safe->next = head;
    head->prev = safe;
    if (descend)
        q_reverse(head);
}

int q_purge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;
    int cnt = 1;
    struct list_head *node = NULL, *safe = NULL, *peak = head->prev;
    for (node = head->prev->prev, safe = node->prev; node != head;
         node = safe, safe = node->prev) {
        const char *s1 = list_entry(peak, element_t, list)->value;
        const char *s2 = list_entry(node, element_t, list)->value;
        if ((!descend && strcmp(s1, s2) <= 0) ||
            (descend && strcmp(s1, s2) >= 0)) {
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
        } else {
            peak = node;
            cnt += 1;
        }
    }
    return cnt;
}
/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    return q_purge(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    return q_purge(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
