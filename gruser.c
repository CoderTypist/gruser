#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LEN_USR 32
#define MAX_LEN_GRP 32


struct Node
{
    void* pValue;
    struct Node* pPrev;
    struct Node* pNext;
};
typedef struct Node Node_t;


struct Group
{
    char* name;
    gid_t gid;
};
typedef struct Group Group_t;


struct User
{
    char* name;
    uid_t uid;
    Node_t* groups;
};
typedef struct User User_t;


User_t* User_from(struct passwd* info);
bool User_similar(void* v1, void* v2); // User_t* == char*
void User_free(void* pVoid);

Group_t* Group_from(struct group* info);
bool Group_equals(void* v1, void* v2);  // Group_t* == Group_t*
bool Group_similar(void* v1, void* v2); // Group_t* == struct group*
void Group_free(void* pVoid);

Node_t* Node_new();
bool Node_contains(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*));
Node_t* Node_get_node(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*));
void* Node_get_value(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*));
void Node_free(Node_t* pHead, void(freeValue)(void*));
Node_t* Node_from(void* pValue);
void Node_append(Node_t** ppHead, void* pValue);

void read_passwd(Node_t** ppUsers, Node_t** ppGroups);
void read_group(Node_t** ppUsers, Node_t** ppGroups);
void show_user_groups(Node_t* user_list);


int
main(int argc, char** argv)
{
    Node_t* user_list = NULL;
    Node_t* group_list = NULL;

    read_passwd(&user_list, &group_list);
    read_group(&user_list, &group_list);
    
    show_user_groups(user_list);

    Node_free(user_list, User_free);
    Node_free(group_list, Group_free);

    return 0;
}


User_t*
User_from(struct passwd* info)
{
    assert(info != NULL);

    User_t* pUser = (User_t*)calloc(1, sizeof(User_t));
    assert(pUser != NULL);

    pUser->name = strndup(info->pw_name, MAX_LEN_USR);
    assert(pUser->name != NULL);
    pUser->uid = info->pw_uid;
    return pUser;
}


bool
User_similar(void* v1, void* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);

    User_t* pUser = (User_t*)v1;
    assert(pUser->name != NULL);

    char* uname = (char*)v2;
    return (0 == strncmp(pUser->name, uname, MAX_LEN_USR)) ? true : false;
}


void
User_free(void* pVoid)
{
    if( !pVoid )
        return;
    User_t* pUser = (User_t*)pVoid;
    if( pUser->name )
        free(pUser->name);
    free(pUser);
}


Group_t*
Group_from(struct group* info)
{
    assert(info != NULL);

    Group_t* pGroup = (Group_t*)calloc(1, sizeof(Group_t));
    assert(pGroup != NULL);

    pGroup->name = strndup(info->gr_name, MAX_LEN_GRP);
    assert(pGroup->name != NULL);
    pGroup->gid = info->gr_gid;
    return pGroup;
}


bool
Group_equals(void* v1, void* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);

    Group_t* g1 = (Group_t*)v1;
    Group_t* g2 = (Group_t*)v2;

    assert(g1->name != NULL);
    assert((g1->name)[0] != '\0');
    assert(g2->name != NULL);
    assert((g2->name)[0] != '\0');

    if( 0 != strncmp(g1->name, g2->name, MAX_LEN_GRP) )
        return false;

    return (g1->gid == g2->gid) ? true : false;
}


bool
Group_similar(void* v1, void* v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);

    Group_t* pGroup = (Group_t*)v1;
    struct group* pInfo = (struct group*)v2;

    if( 0 != strncmp(pGroup->name, pInfo->gr_name, MAX_LEN_GRP) )
        return false;
    return (pGroup->gid == pInfo->gr_gid) ? true : false;
}


void
Group_free(void* pVoid)
{
    if( !pVoid )
        return;
    Group_t* pGroup = (Group_t*)pVoid;
    if( pGroup->name )
        free(pGroup->name);
    free(pGroup);
}


Node_t*
Node_new()
{
    Node_t* pNode = (Node_t*)calloc(1, sizeof(Node_t));
    assert(pNode != NULL);
    return pNode;
}


Node_t*
Node_from(void* pValue)
{
    assert(pValue != NULL);

    Node_t* pNode = Node_new();
    pNode->pValue = pValue;
    return pNode;
}


void
Node_append(Node_t** ppHead, void* pValue)
{
    assert(ppHead != NULL);
    assert(pValue != NULL);

    if( !(*ppHead) )
    {
        *ppHead = Node_from(pValue);
        return;
    }

    Node_t* pHead = *ppHead;
    Node_t* pPrev = pHead;
    Node_t* pCur = pHead->pNext;

    while( pCur )
    {
        pPrev = pCur;
        pCur = pCur->pNext;
    }
    pPrev->pNext = Node_from(pValue);
}


bool
Node_contains(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*))
{
    if( pHead == NULL )
        return false;
    assert(pValue != NULL);
    assert(fnEqual != NULL);

    Node_t* pCur = pHead;
    while( pCur )
    {
        if( fnEqual(pCur->pValue, pValue) )
            return true;
        pCur = pCur->pNext;
    }

    return false;
}


Node_t*
Node_get_node(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*))
{
    if( pHead == NULL )
        return NULL;
    assert(pValue != NULL);
    assert(fnEqual != NULL);

    Node_t* pCur = pHead;
    while( pCur )
    {
        if( fnEqual(pCur->pValue, pValue) )
            return pCur;
        pCur = pCur->pNext;
    }

    return NULL;
}


void*
Node_get_value(Node_t* pHead, void* pValue, bool(fnEqual)(void*, void*))
{
    if( pHead == NULL )
        return NULL;
    assert(pValue != NULL);
    assert(fnEqual != NULL);

    Node_t* pNode = Node_get_node(pHead, pValue, fnEqual);
    return pNode ? pNode->pValue : NULL;
}


void
Node_free(Node_t* pHead, void(fnFree)(void*))
{
    Node_t* pPrev = NULL;
    Node_t* pCur = pHead;

    while( pCur )
    {
        pPrev = pCur;
        pCur = pCur->pNext;

        if( fnFree )
            fnFree(pPrev->pValue);
        free(pPrev);
    }
}


void
read_passwd(Node_t** ppUsers, Node_t** ppGroups)
{
    assert(ppUsers != NULL);

    struct passwd* pInfo;
    errno=0;

    while( NULL != (pInfo=getpwent()) )
    {
        User_t* pUser = User_from(pInfo);
        Node_append(ppUsers, pUser);

        // if maintaining a list of groups
        if( ppGroups )
        {
            Group_t* pGroup = Group_from(getgrgid(pInfo->pw_gid));
            
            if( !Node_contains(*ppGroups, pGroup, Group_equals) )
                Node_append(ppGroups, pGroup);

            Node_append(&(pUser->groups), pGroup);
        }
    }

    if( errno != 0 )
    {
        fprintf(stderr, "\tError: read_passwd(): errno == %d\n", errno);
        exit(-1);
    }
    endpwent();
}


void
read_group(Node_t** ppUsers, Node_t** ppGroups)
{
    assert(ppGroups != NULL);

    struct group* pInfo;
    errno = 0;

    while( NULL != (pInfo=getgrent()) )
    {
        // add group to group list if not already present
        Group_t* pGroup = (Group_t*)Node_get_value(*ppGroups, pInfo, Group_similar);
        if( !pGroup )
        {
            pGroup = Group_from(pInfo);
            Node_append(ppGroups, pGroup);
        }

        // if maintaining a list of users
        if( ppUsers )
        {
            char** pCurMem = pInfo->gr_mem;
            if( pCurMem )
            {
                while( *pCurMem )
                {
                    User_t* pUser = (User_t*)Node_get_value(*ppUsers, *pCurMem, User_similar);
                    if( !pUser )
                        fprintf(stderr, "\tWarning: Group %s (%d) contains User %s which is not in the user list\n", pInfo->gr_name, pInfo->gr_gid, *pCurMem);
                    else
                    {
                        if( !Node_contains(pUser->groups, pGroup, Group_equals) )
                            Node_append(&(pUser->groups), pGroup);
                    }
                    pCurMem++;
                }
            }
        }
    }
}


void
show_user_groups(Node_t* user_list)
{
    printf("Users:\n");
    Node_t* pNodeUser = user_list;
    while( pNodeUser )
    {
        User_t* pUser = (User_t*)(pNodeUser->pValue);
        printf("    %s (%d)\n", pUser->name, pUser->uid);
        
        Node_t* pNodeGroup = pUser->groups;
        while( pNodeGroup )
        {
            Group_t* pGroup = (Group_t*)(pNodeGroup->pValue);
            printf("        %s (%d)\n", pGroup->name, pGroup->gid);
            pNodeGroup = pNodeGroup->pNext;
        }
        pNodeUser = pNodeUser->pNext;
    }
}
