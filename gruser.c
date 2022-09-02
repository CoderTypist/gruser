#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// NOTE: Yes, this could have been done as script, but I wanted to practice it this way.
// NOTE: Yes, a script (or other languages) could have better performance because hashmaps could be used.
// NOTE: Yes, this code style is not the norm, I just wanted to try it out.
// NOTE: This program considers that /etc/passwd and /etc/group are properly formatted.
// NOTE: This program is not thread safe because getpwent() and getgrent() are nonreentrant.
// NOTE: Yes, I could have used getpwent_r() and getgrent_r(), but I didn't want to.
// NOTE: Yes, I only copied a subset of the passwd and group structs, but that is all that was needed.


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
void User_free(void* pVoid);
Group_t* Group_from(struct group* info);
bool Group_equals(void* v1, void* v2);
void Group_free(void* pVoid);

Node_t* Node_new();
void Node_free(Node_t* pHead, void(freeValue)(void*) );
Node_t* Node_from(void* pValue);
void Node_append(Node_t** ppHead, void* pValue);

Node_t* read_passwd(Node_t** ppUsers, Node_t** ppGroups);
Node_t* read_group(Node_t** ppUsers, Node_t** ppGroups);
void show_user_groups(Node_t* user_list, Node_t* group_list);


int
main(int argc, char** argv)
{
    Node_t* user_list = NULL;
    Group_t* group_list = NULL;

    // ...

    return 0;
}


User_t*
User_from(struct passwd* info)
{
    assert(info != NULL);

    User_t* pUser = (User_t*)calloc(1, sizeof(User_t));
    assert(pUser != NULL);

    pUser->name = strndup(info->pw_name, MAX_LEN_USR);
    pUser->uid = info->pw_uid;
    return pUser;
}


void
User_free(void* pVoid)
{
    if( !pVoid )
        return;
    User_t* pUser = (User_t*)pVoid;
    if( pUser->name )
        free(pUser->name);
    if( pUser->groups )
        Node_free(pUser->groups, &Group_free);
    free(pUser);
}


Group_t*
Group_from(struct group* info)
{
    assert(info != NULL);

    Group_t* pGroup = (Group_t*)calloc(1, sizeof(Group_t));
    assert(pGroup != NULL);

    pGroup->name = strndup(info->gr_name, MAX_LEN_GRP);
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
    {
        fprintf(stderr, "\tWarning: Node_contains(): pHead is NULL\n");
        return false;
    }
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


void
Node_free(Node_t* pHead, void(freeValue)(void*))
{
    Node_t* pPrev = NULL;
    Node_t* pCur = pHead;

    while( pCur )
    {
        pPrev = pCur;
        pCur = pCur->pNext;

        if( freeValue )
            freeValue(pPrev->pValue);
        free(pPrev);
    }
}


Node_t*
read_passwd(Node_t** ppUsers, Node_t** ppGroups)
{
    assert(ppUsers != NULL);

    struct passwd* pInfo;
    errno=0;

    while( NULL != (pInfo=getpwent()) )
    {
        Group_t* pGroup = Group_from(getgrgid(pInfo->pw_gid));
        
        if( !Node_contains(*ppGroups, pGroup, &Group_equals) )
            Node_append(ppGroups, pGroup);

        User_t* pUser = User_from(pInfo);
        Node_append(&(pUser->groups), pGroup);
        Node_append(ppUsers, pUser);
    }

    if( errno != 0 )
    {
        fprintf(stderr, "\tError: read_passwd(): errno == %d\n", errno);
        exit(-1);
    }
    endpwent();
}


Node_t*
read_group(Node_t** ppUsers, Node_t** ppGroups)
{
    Node_t* group_list = NULL;
    struct group* pInfo;
    errno = 0;

    while( NULL != (pInfo=getgrent()) )
        Node_append(&group_list, Group_from(pInfo));

    /*
    for g in groups:
        for uname in g->unames:
            for u in users:
                if uname == u->uname:
                    Node_append(u, g)
    */
}


void
show_user_groups(Node_t* user_list, Node_t* group_list)
{
    
}
