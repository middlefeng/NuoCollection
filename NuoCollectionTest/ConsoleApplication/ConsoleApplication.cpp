// ConsoleApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "NuoCollection.h"
#include "Utils/Tree.h"


class TestBinTree : public CollectablebinaryTreeNode
{

public:

    TestBinTree(const std::string& name)
        : CollectablebinaryTreeNode(name)
    {
    }

    ~TestBinTree()
    {
        printf("Destructed %s.\n", Name().c_str());
    }

};


class TestNuoMemberObject;


class TestNuoObject : public NuoObject<TestNuoObject>
{

public:

    std::string _name;

    NuoMemberPtr<TestNuoObject, TestNuoMemberObject> _field1;

    TestNuoObject();

    virtual ~TestNuoObject()
    {
        printf("Test Object Destructed.\n");
    }

};

TestNuoObject::TestNuoObject()
    : _field1(this)
{
}



class TestNuoMemberObject : public NuoObject<TestNuoMemberObject>
{

public:

    NuoMemberPtr<TestNuoMemberObject, TestNuoObject> _parent;

    TestNuoMemberObject()
        : _parent(this)
    {
    }

    virtual ~TestNuoMemberObject()
    {
        printf("Test Member Destructed.\n");
    }

};





int main()
{
    NuoCollection* manager = new NuoCollection();

    NuoStackPtr<TestNuoObject> obj1(new TestNuoObject, manager);
    NuoStackPtr<TestNuoObject> obj2 = obj1;

    NuoStackPtr<TestNuoMemberObject> member(new TestNuoMemberObject(), manager);

    obj1->_name = "abc";
    obj1->_field1 = member;
    member->_parent = obj1;

    member.Reset();

    obj1.Reset();
    obj2.Reset();

    manager->CollectGarbage();

    // Tree

    NuoStackPtr<TestBinTree> root(new TestBinTree("root"), manager);
    NuoStackPtr<TestBinTree> cl(new TestBinTree("cl"), manager);
    NuoStackPtr<TestBinTree> cr(new TestBinTree("cr"), manager);

    root->_l = cl;
    root->_r = cr;

    std::string treeDesc = root->ToString();
    printf("Tree:\n%s", treeDesc.c_str());

    root.Reset();
    cl.Reset();
    cr.Reset();

    manager->CollectGarbage();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
