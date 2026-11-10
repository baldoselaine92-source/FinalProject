#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cctype>
 
using namespace std;
 
// ============================================================================
//  PRODUCT MODEL (required struct)
// ============================================================================
struct Product {
    int    id;             // Unique Identifier
    string name;           // Product Title
    string category;       // Category (e.g., Smartphones, Laptops)
    string brand;          // Official Partner Brand
    double price;          // Unit Price (PHP)
    int    stockQuantity;  // Available Inventory Level
};
 
// ============================================================================
//  UTILITY HELPERS
// ============================================================================
static long g_comparisons = 0;   // comparison counter used to analyze algorithms
 
string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++)
        s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}
 
string trim(const string &s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}
 
string itos(int v) {
    ostringstream oss;
    oss << v;
    return oss.str();
}
 
string truncateText(const string &s, size_t width) {
    if (s.size() <= width) return s;
    return s.substr(0, width - 3) + "...";
}
 
// Formats a number as "PHP 12,345.00"
string fmtMoney(double v) {
    if (v < 0) v = 0;
    long total = (long)(v * 100.0 + 0.5);
    long whole = total / 100;
    int cents = (int)(total % 100);
 
    string digits;
    if (whole == 0) digits = "0";
    while (whole > 0) {
        digits.insert(digits.begin(), (char)('0' + (int)(whole % 10)));
        whole /= 10;
    }
    string out;
    int cnt = 0;
    for (int i = (int)digits.size() - 1; i >= 0; i--) {
        out.insert(out.begin(), digits[i]);
        cnt++;
        if (cnt % 3 == 0 && i > 0) out.insert(out.begin(), ',');
    }
    string c;
    c += (char)('0' + cents / 10);
    c += (char)('0' + cents % 10);
    return "PHP " + out + "." + c;
}
 
string readLine(const string &prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\n[Input closed] Exiting program.\n";
        exit(0);
    }
    return trim(s);
}
 
int readInt(const string &prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) {
            char *end;
            long v = strtol(s.c_str(), &end, 10);
            if (*end == '\0' && v >= lo && v <= hi) return (int)v;
        }
        cout << "  Invalid input. Enter a whole number from " << lo << " to " << hi << ".\n";
    }
}
 
double readDouble(const string &prompt, double lo) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) {
            char *end;
            double v = strtod(s.c_str(), &end);
            if (*end == '\0' && v >= lo) return v;
        }
        cout << "  Invalid input. Enter a number that is at least " << lo << ".\n";
    }
}
 
string readNonEmpty(const string &prompt) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) return s;
        cout << "  This field cannot be empty.\n";
    }
}
 
void pauseScreen() {
    readLine("\nPress Enter to continue...");
}
 
void swapPtr(Product *&a, Product *&b) {
    Product *t = a;
    a = b;
    b = t;
}
 
// ============================================================================
//  LAB 1 - DYNAMIC ALLOCATION: resizable product catalog
// ============================================================================
class Catalog {
private:
    Product *data;
    int count;
    int capacity;
    int nextIdValue;
    int resizeCount;
    bool verbose;
 
    Catalog(const Catalog &);             // non-copyable (owns raw memory)
    Catalog &operator=(const Catalog &);
 
    void resizeTo(int newCap) {
        Product *nd = new Product[newCap];
        for (int i = 0; i < count; i++) nd[i] = data[i];
        delete[] data;
        if (verbose)
            cout << "  [Lab 1] Catalog memory resized: " << capacity << " -> " << newCap << " slots\n";
        data = nd;
        capacity = newCap;
        resizeCount++;
    }
 
public:
    Catalog(int cap) : data(new Product[cap]), count(0), capacity(cap),
                       nextIdValue(1001), resizeCount(0), verbose(false) {}
    ~Catalog() { delete[] data; }
 
    void setVerbose(bool v) { verbose = v; }
 
    void add(const Product &p) {
        if (count == capacity) resizeTo(capacity * 2);      // grow (doubling)
        data[count++] = p;
        if (p.id >= nextIdValue) nextIdValue = p.id + 1;
    }
 
    bool removeById(int id) {
        for (int i = 0; i < count; i++) {
            if (data[i].id == id) {
                for (int j = i; j < count - 1; j++) data[j] = data[j + 1];
                count--;
                if (capacity > 4 && count <= capacity / 4)   // shrink (halving)
                    resizeTo(capacity / 2);
                return true;
            }
        }
        return false;
    }
 
    int indexOf(int id) const {
        for (int i = 0; i < count; i++)
            if (data[i].id == id) return i;
        return -1;
    }
 
    Product &at(int i) { return data[i]; }
    const Product &at(int i) const { return data[i]; }
    int size() const { return count; }
    int cap() const { return capacity; }
    int resizes() const { return resizeCount; }
    int nextId() const { return nextIdValue; }
};
 
// ============================================================================
//  LAB 6 - BINARY SEARCH TREE: Product ID index
// ============================================================================
struct BSTNode {
    int id;
    int index;               // position of the product inside the Catalog array
    BSTNode *left;
    BSTNode *right;
    BSTNode(int i, int idx) : id(i), index(idx), left(NULL), right(NULL) {}
};
 
class BST {
private:
    BSTNode *root;
 
    void destroy(BSTNode *n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
    BSTNode *insertNode(BSTNode *n, int id, int idx) {
        if (!n) return new BSTNode(id, idx);
        if (id < n->id) n->left = insertNode(n->left, id, idx);
        else            n->right = insertNode(n->right, id, idx);
        return n;
    }
    // Inserting the middle element first keeps the tree balanced
    void buildBalanced(int *ids, int *idxs, int lo, int hi) {
        if (lo > hi) return;
        int mid = (lo + hi) / 2;
        root = insertNode(root, ids[mid], idxs[mid]);
        buildBalanced(ids, idxs, lo, mid - 1);
        buildBalanced(ids, idxs, mid + 1, hi);
    }
    int heightOf(BSTNode *n) const {
        if (!n) return 0;
        int l = heightOf(n->left), r = heightOf(n->right);
        return 1 + (l > r ? l : r);
    }
 
public:
    BST() : root(NULL) {}
    ~BST() { destroy(root); }
 
    void rebuild(const Catalog &c) {
        destroy(root);
        root = NULL;
        int n = c.size();
        if (n == 0) return;
        int *ids = new int[n];
        int *idxs = new int[n];
        for (int i = 0; i < n; i++) { ids[i] = c.at(i).id; idxs[i] = i; }
        for (int i = 1; i < n; i++) {                       // sort IDs (insertion sort)
            int k = ids[i], ki = idxs[i], j = i - 1;
            while (j >= 0 && ids[j] > k) { ids[j + 1] = ids[j]; idxs[j + 1] = idxs[j]; j--; }
            ids[j + 1] = k;
            idxs[j + 1] = ki;
        }
        buildBalanced(ids, idxs, 0, n - 1);
        delete[] ids;
        delete[] idxs;
    }
 
    // returns catalog index, or -1; steps = number of nodes visited
    int find(int id, int &steps) const {
        BSTNode *n = root;
        steps = 0;
        while (n) {
            steps++;
            if (id == n->id) return n->index;
            n = (id < n->id) ? n->left : n->right;
        }
        return -1;
    }
    int height() const { return heightOf(root); }
};
 
// ============================================================================
//  LAB 2 - DOUBLY LINKED LIST: shopping cart with next/previous navigation
// ============================================================================
struct CartNode {
    int productId;
    string name;
    string brand;
    double price;
    int qty;
    CartNode *prev;
    CartNode *next;
};
 
class Cart {
private:
    CartNode *head;
    CartNode *tail;
    CartNode *cur;
    int items;
 
    Cart(const Cart &);
    Cart &operator=(const Cart &);
 
public:
    Cart() : head(NULL), tail(NULL), cur(NULL), items(0) {}
    ~Cart() { clear(); }
 
    bool empty() const { return head == NULL; }
    int size() const { return items; }
    CartNode *first() const { return head; }
    CartNode *current() const { return cur; }
 
    int qtyOf(int productId) const {
        for (CartNode *n = head; n; n = n->next)
            if (n->productId == productId) return n->qty;
        return 0;
    }
 
    void add(const Product &p, int qty) {
        for (CartNode *n = head; n; n = n->next) {           // same item -> merge quantity
            if (n->productId == p.id) { n->qty += qty; cur = n; return; }
        }
        CartNode *n = new CartNode;
        n->productId = p.id;
        n->name = p.name;
        n->brand = p.brand;
        n->price = p.price;
        n->qty = qty;
        n->next = NULL;
        n->prev = tail;
        if (tail) tail->next = n; else head = n;
        tail = n;
        cur = n;
        items++;
    }
 
    bool moveNext() {
        if (!cur || !cur->next) return false;
        cur = cur->next;
        return true;
    }
    bool movePrev() {
        if (!cur || !cur->prev) return false;
        cur = cur->prev;
        return true;
    }
 
    bool removeCurrent() {
        if (!cur) return false;
        CartNode *victim = cur;
        if (victim->prev) victim->prev->next = victim->next; else head = victim->next;
        if (victim->next) victim->next->prev = victim->prev; else tail = victim->prev;
        cur = victim->next ? victim->next : victim->prev;
        delete victim;
        items--;
        return true;
    }
 
    int positionOfCurrent() const {
        int pos = 0;
        for (CartNode *n = head; n; n = n->next) {
            pos++;
            if (n == cur) return pos;
        }
        return 0;
    }
 
    double total() const {
        double t = 0;
        for (CartNode *n = head; n; n = n->next) t += n->price * n->qty;
        return t;
    }
 
    void clear() {
        while (head) {
            CartNode *n = head;
            head = head->next;
            delete n;
        }
        tail = cur = NULL;
        items = 0;
    }
};
 
// ============================================================================
//  LAB 3 - CIRCULAR LINKED LIST: featured brand banners
// ============================================================================
struct BrandNode {
    string brand;
    string tagline;
    BrandNode *next;
};
 
class BrandRing {
private:
    BrandNode *head;
    BrandNode *tail;
    int count;
 
    BrandRing(const BrandRing &);
    BrandRing &operator=(const BrandRing &);
 
public:
    BrandRing() : head(NULL), tail(NULL), count(0) {}
    ~BrandRing() {
        BrandNode *cur = head;
        for (int i = 0; i < count; i++) {
            BrandNode *nx = cur->next;
            delete cur;
            cur = nx;
        }
    }
 
    void add(const string &brand, const string &tagline) {
        BrandNode *n = new BrandNode;
        n->brand = brand;
        n->tagline = tagline;
        if (!head) {
            head = tail = n;
            n->next = n;                 // points to itself
        } else {
            tail->next = n;
            n->next = head;              // last node links back to the first
            tail = n;
        }
        count++;
    }
 
    BrandNode *first() const { return head; }
    int size() const { return count; }
 
    // canonical (stored) spelling of a brand, or "" if it is not a partner
    string canonical(const string &name) const {
        string key = toLower(name);
        BrandNode *n = head;
        for (int i = 0; i < count; i++, n = n->next)
            if (toLower(n->brand) == key) return n->brand;
        return "";
    }
};
 
// ============================================================================
//  LAB 4 - STACK (LIFO): admin undo / redo
// ============================================================================
enum ActionType { ACT_ADD = 1, ACT_REMOVE = 2, ACT_STOCK = 3 };
 
struct Action {
    int type;
    int group;          // actions of one order/operation share a group id
    Product product;    // snapshot of the affected product
    int oldStock;
    int newStock;
    string desc;
    Action() : type(0), group(0), oldStock(0), newStock(0) {
        product.id = 0; product.price = 0; product.stockQuantity = 0;
    }
};
 
struct StackNode {
    Action a;
    StackNode *next;
    StackNode(const Action &x) : a(x), next(NULL) {}
};
 
class ActionStack {
private:
    StackNode *topNode;
    int count;
 
    ActionStack(const ActionStack &);
    ActionStack &operator=(const ActionStack &);
 
public:
    ActionStack() : topNode(NULL), count(0) {}
    ~ActionStack() { clear(); }
 
    void push(const Action &a) {
        StackNode *n = new StackNode(a);
        n->next = topNode;
        topNode = n;
        count++;
    }
    bool pop(Action &out) {
        if (!topNode) return false;
        StackNode *n = topNode;
        out = n->a;
        topNode = n->next;
        delete n;
        count--;
        return true;
    }
    bool peekGroup(int &g) const {
        if (!topNode) return false;
        g = topNode->a.group;
        return true;
    }
    bool empty() const { return topNode == NULL; }
    int size() const { return count; }
    void clear() {
        Action tmp;
        while (pop(tmp)) {}
    }
    void printTop(int maxLines) const {
        int shown = 0;
        for (StackNode *n = topNode; n && shown < maxLines; n = n->next, shown++)
            cout << "  " << (shown == 0 ? "TOP -> " : "       ") << n->a.desc << "\n";
        if (count > maxLines) cout << "         ... and " << (count - maxLines) << " older action(s)\n";
    }
};
 
// ============================================================================
//  LAB 5 - QUEUE (FIFO): customer checkout / order processing line
// ============================================================================
struct OrderLine {
    int productId;
    string name;
    double price;
    int qty;
};
 
struct OrderNode {
    int orderId;
    string customer;
    OrderLine *lines;        // dynamic array of purchased lines
    int lineCount;
    double total;
    OrderNode *next;
    OrderNode() : orderId(0), lines(NULL), lineCount(0), total(0), next(NULL) {}
    ~OrderNode() { delete[] lines; }
};
 
class OrderQueue {
private:
    OrderNode *frontNode;
    OrderNode *rearNode;
    int count;
 
    OrderQueue(const OrderQueue &);
    OrderQueue &operator=(const OrderQueue &);
 
public:
    OrderQueue() : frontNode(NULL), rearNode(NULL), count(0) {}
    ~OrderQueue() {
        while (frontNode) {
            OrderNode *n = frontNode;
            frontNode = frontNode->next;
            delete n;
        }
    }
 
    void enqueue(OrderNode *n) {
        n->next = NULL;
        if (rearNode) rearNode->next = n; else frontNode = n;
        rearNode = n;
        count++;
    }
    OrderNode *dequeue() {          // caller owns (and deletes) the returned node
        if (!frontNode) return NULL;
        OrderNode *n = frontNode;
        frontNode = frontNode->next;
        if (!frontNode) rearNode = NULL;
        n->next = NULL;
        count--;
        return n;
    }
    bool empty() const { return frontNode == NULL; }
    int size() const { return count; }
    OrderNode *front() const { return frontNode; }
};
 
// ============================================================================
//  LAB 8 - SORTING ALGORITHMS (operate on arrays of Product pointers)
// ============================================================================
enum SortKey { KEY_PRICE = 1, KEY_STOCK = 2, KEY_BRAND = 3 };
 
int compareProducts(const Product *a, const Product *b, int key) {
    g_comparisons++;
    if (key == KEY_PRICE) {
        if (a->price < b->price) return -1;
        if (a->price > b->price) return 1;
        return 0;
    }
    if (key == KEY_STOCK) {
        if (a->stockQuantity < b->stockQuantity) return -1;
        if (a->stockQuantity > b->stockQuantity) return 1;
        return 0;
    }
    string x = toLower(a->brand), y = toLower(b->brand);     // brand, then name
    if (x < y) return -1;
    if (x > y) return 1;
    string nx = toLower(a->name), ny = toLower(b->name);
    if (nx < ny) return -1;
    if (nx > ny) return 1;
    return 0;
}
 
void insertionSort(Product **a, int n, int key) {            // stable, O(n^2)
    for (int i = 1; i < n; i++) {
        Product *k = a[i];
        int j = i - 1;
        while (j >= 0 && compareProducts(a[j], k, key) > 0) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = k;
    }
}
 
void quickSort(Product **a, int lo, int hi, int key) {       // average O(n log n)
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    swapPtr(a[mid], a[hi]);                                  // middle element as pivot
    Product *pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (compareProducts(a[j], pivot, key) <= 0) {
            i++;
            swapPtr(a[i], a[j]);
        }
    }
    swapPtr(a[i + 1], a[hi]);
    int p = i + 1;
    quickSort(a, lo, p - 1, key);
    quickSort(a, p + 1, hi, key);
}
 
void reverseView(Product **a, int n) {
    for (int i = 0, j = n - 1; i < j; i++, j--) swapPtr(a[i], a[j]);
}
 
// ============================================================================
//  TABLE DISPLAY
// ============================================================================
void tableLine() {
    cout << "+-------+----------------------------+--------------+----------+----------------+-------+\n";
}
 
void printTable(Product **v, int n) {
    tableLine();
    cout << "| " << left << setw(5) << "ID" << " | " << setw(26) << "Product Name"
         << " | " << setw(12) << "Category" << " | " << setw(8) << "Brand"
         << " | " << right << setw(14) << "Price" << " | " << setw(5) << "Stock" << " |\n";
    tableLine();
    if (n == 0) {
        cout << "|                              (no products to display)                                 |\n";
    }
    for (int i = 0; i < n; i++) {
        const Product *p = v[i];
        cout << "| " << left << setw(5) << p->id << " | " << setw(26) << truncateText(p->name, 26)
             << " | " << setw(12) << truncateText(p->category, 12)
             << " | " << setw(8) << truncateText(p->brand, 8)
             << " | " << right << setw(14) << fmtMoney(p->price)
             << " | " << setw(5) << p->stockQuantity << " |\n";
    }
    tableLine();
    cout << left;
}
 
void printProductCard(const Product &p) {
    cout << "  ID       : " << p.id << "\n"
         << "  Name     : " << p.name << "\n"
         << "  Category : " << p.category << "\n"
         << "  Brand    : " << p.brand << "\n"
         << "  Price    : " << fmtMoney(p.price) << "\n"
         << "  Stock    : " << p.stockQuantity << "\n";
}
 
// ============================================================================
//  STORE: ties every data structure together and implements the menu
// ============================================================================
class Store {
private:
    Catalog catalog;
    BST index;
    BrandRing brands;
    Cart cart;
    OrderQueue orders;
    ActionStack undoStack;
    ActionStack redoStack;
    int nextOrderId;
    int nextGroup;
 
    Store(const Store &);
    Store &operator=(const Store &);
 
    // ---- helpers ----------------------------------------------------------
    Product **makeView(int &n) {
        n = catalog.size();
        Product **v = new Product *[n > 0 ? n : 1];
        for (int i = 0; i < n; i++) v[i] = &catalog.at(i);
        return v;
    }
 
    void addSeed(const char *name, const char *cat, const char *brand, double price, int stock) {
        Product p;
        p.id = catalog.nextId();
        p.name = name;
        p.category = cat;
        p.brand = brand;
        p.price = price;
        p.stockQuantity = stock;
        catalog.add(p);
    }
 
    void recordAction(const Action &a) {
        undoStack.push(a);
        redoStack.clear();               // a new action invalidates the redo history
    }
 
    void applyUndo(const Action &a) {
        if (a.type == ACT_ADD) {
            catalog.removeById(a.product.id);
            index.rebuild(catalog);
        } else if (a.type == ACT_REMOVE) {
            catalog.add(a.product);
            index.rebuild(catalog);
        } else if (a.type == ACT_STOCK) {
            int i = catalog.indexOf(a.product.id);
            if (i >= 0) catalog.at(i).stockQuantity = a.oldStock;
        }
    }
 
    void applyRedo(const Action &a) {
        if (a.type == ACT_ADD) {
            catalog.add(a.product);
            index.rebuild(catalog);
        } else if (a.type == ACT_REMOVE) {
            catalog.removeById(a.product.id);
            index.rebuild(catalog);
        } else if (a.type == ACT_STOCK) {
            int i = catalog.indexOf(a.product.id);
            if (i >= 0) catalog.at(i).stockQuantity = a.newStock;
        }
    }
 
    // lets the user pick a partner brand by number or by name; "" if invalid
    string pickBrand() {
        int n = brands.size();
        cout << "\nOfficial partner brands:\n";
        BrandNode *b = brands.first();
        for (int i = 1; i <= n; i++, b = b->next) cout << "  " << i << ". " << b->brand << "\n";
        string in = readLine("Enter brand name or number: ");
        if (in.empty()) return "";
        char *end;
        long num = strtol(in.c_str(), &end, 10);
        if (*end == '\0') {
            if (num < 1 || num > n) return "";
            b = brands.first();
            for (int i = 1; i < num; i++) b = b->next;
            return b->brand;
        }
        return brands.canonical(in);
    }
 
public:
    Store() : catalog(4), nextOrderId(5001), nextGroup(1) {}
 
    // ---- initial data: Tech & Electronics, 4 partner brands ---------------
    void seed() {
        brands.add("Apple",   "Think different. Premium devices for everyone.");
        brands.add("Samsung", "Innovation that opens new possibilities.");
        brands.add("Sony",    "Be moved. Sound, picture and play in one brand.");
        brands.add("ASUS",    "In search of incredible. Laptops and gaming gear.");
 
        addSeed("iPhone 15 128GB",          "Smartphones", "Apple",   52990.00, 14);
        addSeed("iPad Air 11-inch",         "Tablets",     "Apple",   36990.00,  9);
        addSeed("MacBook Air M2",           "Laptops",     "Apple",   63990.00,  6);
        addSeed("AirPods Pro (2nd gen)",    "Audio",       "Apple",   14990.00, 25);
        addSeed("Apple Watch SE",           "Wearables",   "Apple",   15990.00, 12);
 
        addSeed("Galaxy S24 256GB",         "Smartphones", "Samsung", 49990.00, 15);
        addSeed("Galaxy Tab S9",            "Tablets",     "Samsung", 44990.00,  8);
        addSeed("Galaxy Buds2 Pro",         "Audio",       "Samsung",  9990.00, 30);
        addSeed("Galaxy Watch6",            "Wearables",   "Samsung", 17990.00, 10);
        addSeed("Odyssey G5 27-inch",       "Monitors",    "Samsung", 15990.00,  7);
 
        addSeed("WH-1000XM5 Headphones",    "Audio",       "Sony",    21990.00, 18);
        addSeed("PlayStation 5 Slim",       "Gaming",      "Sony",    32990.00,  5);
        addSeed("Alpha ZV-E10 Camera",      "Cameras",     "Sony",    42990.00,  4);
        addSeed("Bravia 43-inch 4K TV",     "TVs",         "Sony",    27990.00,  6);
        addSeed("SRS-XB13 Speaker",         "Audio",       "Sony",     3990.00, 40);
 
        addSeed("ROG Strix G16",            "Laptops",     "ASUS",    89990.00,  3);
        addSeed("Zenbook 14 OLED",          "Laptops",     "ASUS",    59990.00,  7);
        addSeed("ROG Ally Handheld",        "Gaming",      "ASUS",    34990.00,  5);
        addSeed("TUF VG259 Monitor",        "Monitors",    "ASUS",     8990.00, 11);
        addSeed("RT-AX55 WiFi Router",      "Networking",  "ASUS",     3290.00, 20);
 
        index.rebuild(catalog);
        catalog.setVerbose(true);        // from now on, show every memory resize
        cout << "  [Lab 1] Catalog loaded: " << catalog.size() << " products, capacity "
             << catalog.cap() << " (resized " << catalog.resizes() << " times while loading)\n";
        cout << "  [Lab 6] BST index built: height " << index.height() << "\n";
    }
 
    // ---- 1. View complete catalog ----------------------------------------
    void viewCatalog() {
        int n;
        Product **v = makeView(n);
        cout << "\n=== COMPLETE CATALOG (Tech & Electronics) ===\n";
        printTable(v, n);
        cout << "Total products: " << n << "  |  Array capacity: " << catalog.cap()
             << " slots  |  BST height: " << index.height() << "\n";
        delete[] v;
    }
 
    // ---- 2. Filter by brand ----------------------------------------------
    void filterByBrand() {
        string brand = pickBrand();
        if (brand.empty()) {
            cout << "  That is not one of the official partner brands.\n";
            return;
        }
        int n;
        Product **all = makeView(n);
        Product **hit = new Product *[n > 0 ? n : 1];
        int m = 0, units = 0;
        for (int i = 0; i < n; i++) {
            if (toLower(all[i]->brand) == toLower(brand)) {
                hit[m++] = all[i];
                units += all[i]->stockQuantity;
            }
        }
        cout << "\n=== PRODUCTS BY " << brand << " ===\n";
        printTable(hit, m);
        cout << m << " product(s) found  |  " << units << " unit(s) in stock\n";
        delete[] all;
        delete[] hit;
    }
 
    // ---- 3. Search (linear keyword + BST by ID) -------------------------
    void searchMenu() {
        cout << "\n--- SEARCH ---\n"
             << "1. Keyword search (Linear Search)\n"
             << "2. Product ID search (BST, O(log N))\n"
             << "0. Back\n";
        int c = readInt("Choose: ", 0, 2);
        if (c == 1) keywordSearch();
        else if (c == 2) idSearch();
    }
 
    void keywordSearch() {
        string kw = toLower(readNonEmpty("Enter keyword (name, brand or category): "));
        int n;
        Product **all = makeView(n);
        Product **hit = new Product *[n > 0 ? n : 1];
        int m = 0;
        long scanned = 0;
        for (int i = 0; i < n; i++) {                      // linear scan: check every record
            scanned++;
            string hay = toLower(all[i]->name + " " + all[i]->brand + " " + all[i]->category);
            if (hay.find(kw) != string::npos) hit[m++] = all[i];
        }
        cout << "\n=== SEARCH RESULTS FOR \"" << kw << "\" ===\n";
        printTable(hit, m);
        cout << "[Lab 7] Linear search scanned " << scanned << " of " << n
             << " records  |  " << m << " match(es)\n";
        delete[] all;
        delete[] hit;
    }
 
    void idSearch() {
        int id = readInt("Enter Product ID: ", 1, 999999999);
        int steps;
        int idx = index.find(id, steps);
        if (idx < 0) {
            cout << "  Product ID " << id << " not found (BST checked " << steps << " node(s)).\n";
            return;
        }
        cout << "\n=== PRODUCT FOUND ===\n";
        printProductCard(catalog.at(idx));
        cout << "[Lab 6] BST visited " << steps << " node(s); a linear scan could need up to "
             << catalog.size() << ".\n";
    }
 
    // ---- 4. Price range (binary search) ----------------------------------
    void filterByPrice() {
        double lo = readDouble("Minimum price (PHP): ", 0);
        double hi = readDouble("Maximum price (PHP): ", lo);
 
        int n;
        Product **v = makeView(n);
        g_comparisons = 0;
        quickSort(v, 0, n - 1, KEY_PRICE);                 // binary search needs sorted data
        long sortCmp = g_comparisons;
 
        long steps = 0;
        int a = 0, b = n;                                  // first index with price >= lo
        while (a < b) {
            steps++;
            int mid = (a + b) / 2;
            if (v[mid]->price < lo) a = mid + 1; else b = mid;
        }
        int start = a;
        a = start; b = n;                                  // first index with price > hi
        while (a < b) {
            steps++;
            int mid = (a + b) / 2;
            if (v[mid]->price <= hi) a = mid + 1; else b = mid;
        }
        int end = a;
 
        cout << "\n=== PRODUCTS FROM " << fmtMoney(lo) << " TO " << fmtMoney(hi) << " ===\n";
        printTable(v + start, end - start);
        cout << (end - start) << " product(s) in range\n"
             << "[Lab 8] Quick Sort prepared the price view (" << sortCmp << " comparisons)\n"
             << "[Lab 7] Binary Search located both range boundaries in " << steps << " step(s)\n";
        delete[] v;
    }
 
    // ---- 5. Shopping cart (doubly linked list) -----------------------------
    void showCurrentCartItem() {
        CartNode *c = cart.current();
        if (!c) {
            cout << "  The cart is empty.\n";
            return;
        }
        cout << "  Item " << cart.positionOfCurrent() << " of " << cart.size() << ": "
             << c->name << " (" << c->brand << ")  x" << c->qty
             << "  @ " << fmtMoney(c->price) << "  = " << fmtMoney(c->price * c->qty) << "\n";
    }
 
    void viewCart() {
        cout << "\n=== SHOPPING CART ===\n";
        if (cart.empty()) {
            cout << "  (empty)\n";
            return;
        }
        int pos = 0;
        for (CartNode *n = cart.first(); n; n = n->next) {
            pos++;
            cout << (n == cart.current() ? " -> " : "    ") << pos << ". "
                 << left << setw(24) << truncateText(n->name, 24)
                 << setw(9) << n->brand << "x" << setw(4) << n->qty
                 << right << setw(15) << fmtMoney(n->price * n->qty) << left << "\n";
        }
        cout << "    ----------------------------------------------------------\n"
             << "    CART TOTAL: " << fmtMoney(cart.total()) << "   (-> marks the current item)\n";
    }
 
    void cartMenu() {
        while (true) {
            cout << "\n--- SHOPPING CART (Doubly Linked List) ---\n"
                 << "1. Add item by Product ID\n"
                 << "2. Next item  (->)\n"
                 << "3. Previous item  (<-)\n"
                 << "4. Remove current item\n"
                 << "5. View whole cart\n"
                 << "6. Clear cart\n"
                 << "0. Back to main menu\n";
            int c = readInt("Choose: ", 0, 6);
            if (c == 0) return;
            if (c == 1) {
                int id = readInt("Product ID to add: ", 1, 999999999);
                int steps;
                int idx = index.find(id, steps);
                if (idx < 0) { cout << "  Product ID not found.\n"; continue; }
                Product &p = catalog.at(idx);
                int available = p.stockQuantity - cart.qtyOf(p.id);
                cout << "  " << p.name << " (" << p.brand << ") - " << fmtMoney(p.price)
                     << " - " << available << " available\n";
                if (available <= 0) { cout << "  Sorry, no more stock can be added.\n"; continue; }
                int q = readInt("Quantity: ", 1, available);
                cart.add(p, q);
                cout << "  Added to cart.\n";
                showCurrentCartItem();
            } else if (c == 2) {
                if (cart.empty()) cout << "  The cart is empty.\n";
                else if (!cart.moveNext()) cout << "  Already at the last item.\n";
                showCurrentCartItem();
            } else if (c == 3) {
                if (cart.empty()) cout << "  The cart is empty.\n";
                else if (!cart.movePrev()) cout << "  Already at the first item.\n";
                showCurrentCartItem();
            } else if (c == 4) {
                if (cart.removeCurrent()) { cout << "  Item removed.\n"; showCurrentCartItem(); }
                else cout << "  The cart is empty.\n";
            } else if (c == 5) {
                viewCart();
            } else if (c == 6) {
                cart.clear();
                cout << "  Cart cleared.\n";
            }
        }
    }
 
    // ---- 6. Checkout queue (FIFO) -----------------------------------------
    void checkoutCart() {
        if (cart.empty()) {
            cout << "  Your cart is empty. Add items first (menu option 5).\n";
            return;
        }
        string customer = readNonEmpty("Customer name: ");
        OrderNode *o = new OrderNode;
        o->orderId = nextOrderId++;
        o->customer = customer;
        o->lineCount = cart.size();
        o->lines = new OrderLine[o->lineCount];
        int i = 0;
        for (CartNode *n = cart.first(); n; n = n->next, i++) {
            o->lines[i].productId = n->productId;
            o->lines[i].name = n->name;
            o->lines[i].price = n->price;
            o->lines[i].qty = n->qty;
            o->total += n->price * n->qty;
        }
        orders.enqueue(o);
        cart.clear();
        cout << "  Order #" << o->orderId << " placed for " << customer << " (total "
             << fmtMoney(o->total) << "). Position in line: " << orders.size() << "\n";
    }
 
    void viewOrderQueue() {
        cout << "\n=== ORDER QUEUE (front = next to be served) ===\n";
        if (orders.empty()) {
            cout << "  (no pending orders)\n";
            return;
        }
        int pos = 0;
        for (OrderNode *n = orders.front(); n; n = n->next) {
            pos++;
            cout << "  " << pos << ". Order #" << n->orderId << "  " << left << setw(14) << n->customer
                 << n->lineCount << " line(s)  " << fmtMoney(n->total) << "\n";
        }
    }
 
    bool processNextOrder() {
        OrderNode *o = orders.dequeue();
        if (!o) {
            cout << "  No pending orders in the queue.\n";
            return false;
        }
        // 1) verify that every line can still be fulfilled
        for (int i = 0; i < o->lineCount; i++) {
            int steps;
            int idx = index.find(o->lines[i].productId, steps);
            if (idx < 0 || catalog.at(idx).stockQuantity < o->lines[i].qty) {
                cout << "  Order #" << o->orderId << " REJECTED: not enough stock for "
                     << o->lines[i].name << ".\n";
                delete o;
                return true;
            }
        }
        // 2) deduct stock and push each change onto the undo stack
        int group = nextGroup++;
        cout << "\n  ---------- RECEIPT: Order #" << o->orderId << " ----------\n"
             << "  Customer: " << o->customer << "\n";
        for (int i = 0; i < o->lineCount; i++) {
            int steps;
            int idx = index.find(o->lines[i].productId, steps);
            Product &p = catalog.at(idx);
            Action a;
            a.type = ACT_STOCK;
            a.group = group;
            a.product = p;
            a.oldStock = p.stockQuantity;
            a.newStock = p.stockQuantity - o->lines[i].qty;
            a.desc = "Order #" + itos(o->orderId) + ": sold " + itos(o->lines[i].qty) + " x " + p.name
                     + " (stock " + itos(a.oldStock) + " -> " + itos(a.newStock) + ")";
            p.stockQuantity = a.newStock;
            recordAction(a);
            cout << "  " << o->lines[i].qty << " x " << left << setw(24) << truncateText(o->lines[i].name, 24)
                 << right << setw(15) << fmtMoney(o->lines[i].price * o->lines[i].qty) << left << "\n";
        }
        cout << "  TOTAL PAID: " << fmtMoney(o->total) << "\n  Status: PROCESSED\n";
        delete o;
        return true;
    }
 
    void checkoutMenu() {
        while (true) {
            cout << "\n--- CHECKOUT & ORDER QUEUE (FIFO) --- pending orders: " << orders.size() << "\n"
                 << "1. Check out my cart (join the queue)\n"
                 << "2. Process next order (dequeue)\n"
                 << "3. Process ALL pending orders\n"
                 << "4. View order queue\n"
                 << "0. Back to main menu\n";
            int c = readInt("Choose: ", 0, 4);
            if (c == 0) return;
            if (c == 1) checkoutCart();
            else if (c == 2) processNextOrder();
            else if (c == 3) {
                if (orders.empty()) cout << "  No pending orders in the queue.\n";
                while (!orders.empty()) processNextOrder();
            } else if (c == 4) viewOrderQueue();
        }
    }
 
    // ---- 7. Admin actions with Undo/Redo (LIFO stacks) -------------------
    void adminAddProduct() {
        Product p;
        p.id = catalog.nextId();
        p.name = readNonEmpty("Product name: ");
        p.category = readNonEmpty("Category: ");
        p.brand = pickBrand();
        if (p.brand.empty()) { cout << "  Products must belong to an official partner brand. Cancelled.\n"; return; }
        p.price = readDouble("Unit price (PHP): ", 0.01);
        p.stockQuantity = readInt("Initial stock: ", 0, 1000000);
        catalog.add(p);
        index.rebuild(catalog);
        Action a;
        a.type = ACT_ADD;
        a.group = nextGroup++;
        a.product = p;
        a.desc = "Added product #" + itos(p.id) + " " + p.name;
        recordAction(a);
        cout << "  Product added with ID " << p.id << ".\n";
    }
 
    void adminSetStock() {
        int id = readInt("Product ID: ", 1, 999999999);
        int steps;
        int idx = index.find(id, steps);
        if (idx < 0) { cout << "  Product ID not found.\n"; return; }
        Product &p = catalog.at(idx);
        cout << "  " << p.name << " currently has " << p.stockQuantity << " in stock.\n";
        int ns = readInt("New stock level: ", 0, 1000000);
        Action a;
        a.type = ACT_STOCK;
        a.group = nextGroup++;
        a.product = p;
        a.oldStock = p.stockQuantity;
        a.newStock = ns;
        a.desc = "Stock of " + p.name + ": " + itos(a.oldStock) + " -> " + itos(ns);
        p.stockQuantity = ns;
        recordAction(a);
        cout << "  Stock updated.\n";
    }
 
    void adminRemoveProduct() {
        int id = readInt("Product ID to remove: ", 1, 999999999);
        int steps;
        int idx = index.find(id, steps);
        if (idx < 0) { cout << "  Product ID not found.\n"; return; }
        Product p = catalog.at(idx);
        Action a;
        a.type = ACT_REMOVE;
        a.group = nextGroup++;
        a.product = p;
        a.desc = "Removed product #" + itos(p.id) + " " + p.name;
        catalog.removeById(id);
        index.rebuild(catalog);
        recordAction(a);
        cout << "  Removed " << p.name << " from the catalog.\n";
    }
 
    void undoLast() {
        int g;
        if (!undoStack.peekGroup(g)) { cout << "  Nothing to undo.\n"; return; }
        Action a;
        int guard;
        while (undoStack.peekGroup(guard) && guard == g) {   // pop the whole group (LIFO)
            undoStack.pop(a);
            applyUndo(a);
            redoStack.push(a);
            cout << "  UNDONE: " << a.desc << "\n";
        }
    }
 
    void redoLast() {
        int g;
        if (!redoStack.peekGroup(g)) { cout << "  Nothing to redo.\n"; return; }
        Action a;
        int guard;
        while (redoStack.peekGroup(guard) && guard == g) {
            redoStack.pop(a);
            applyRedo(a);
            undoStack.push(a);
            cout << "  REDONE: " << a.desc << "\n";
        }
    }
 
    void adminMenu() {
        while (true) {
            cout << "\n--- ADMIN PANEL (Undo/Redo Stacks) --- undo: " << undoStack.size()
                 << "  redo: " << redoStack.size() << "\n"
                 << "1. Add new product\n"
                 << "2. Adjust stock level\n"
                 << "3. Remove product\n"
                 << "4. UNDO last action (stack pop)\n"
                 << "5. REDO last undone action\n"
                 << "6. View undo history\n"
                 << "0. Back to main menu\n";
            int c = readInt("Choose: ", 0, 6);
            if (c == 0) return;
            if (c == 1) adminAddProduct();
            else if (c == 2) adminSetStock();
            else if (c == 3) adminRemoveProduct();
            else if (c == 4) undoLast();
            else if (c == 5) redoLast();
            else if (c == 6) {
                cout << "\n=== UNDO STACK (most recent on top) ===\n";
                if (undoStack.empty()) cout << "  (empty)\n";
                else undoStack.printTop(10);
            }
        }
    }
 
    // ---- 8. Brand banner showcase (circular linked list) -----------------
    void brandShowcase() {
        BrandNode *cur = brands.first();
        if (!cur) { cout << "  No partner brands registered.\n"; return; }
        int pos = 1;
        int n;
        Product **all = makeView(n);
        while (true) {
            int m = 0, units = 0;
            double lo = 0, hi = 0;
            for (int i = 0; i < n; i++) {
                if (toLower(all[i]->brand) == toLower(cur->brand)) {
                    if (m == 0 || all[i]->price < lo) lo = all[i]->price;
                    if (m == 0 || all[i]->price > hi) hi = all[i]->price;
                    units += all[i]->stockQuantity;
                    m++;
                }
            }
            cout << "\n  ==============================================================\n"
                 << "   FEATURED PARTNER " << pos << "/" << brands.size() << " :  " << cur->brand << "\n"
                 << "   \"" << cur->tagline << "\"\n"
                 << "   " << m << " products  |  " << units << " units in stock\n";
            if (m > 0) cout << "   Price range: " << fmtMoney(lo) << " to " << fmtMoney(hi) << "\n";
            cout << "  ==============================================================\n";
 
            string in = readLine("  [Enter] next banner   |   [q] back to menu: ");
            if (toLower(in) == "q") break;
            cur = cur->next;                                  // circular: last -> first
            pos++;
            if (pos > brands.size()) {
                pos = 1;
                cout << "\n  >> Reached the end of the list. The circular list wraps back to the first banner.\n";
            }
        }
        delete[] all;
    }
 
    // ---- 9. Sort inventory view ------------------------------------------
    void sortInventory() {
        cout << "\n--- SORT INVENTORY ---\n"
             << "1. By Price  (Quick Sort)\n"
             << "2. By Stock  (Insertion Sort)\n"
             << "3. By Brand  (Insertion Sort, then by name)\n"
             << "0. Back\n";
        int key = readInt("Choose: ", 0, 3);
        if (key == 0) return;
        int dir = readInt("1 = Ascending, 2 = Descending: ", 1, 2);
 
        int n;
        Product **v = makeView(n);
        g_comparisons = 0;
        string algo;
        if (key == KEY_PRICE) { quickSort(v, 0, n - 1, key); algo = "Quick Sort"; }
        else                  { insertionSort(v, n, key);    algo = "Insertion Sort"; }
        if (dir == 2) reverseView(v, n);
 
        string by = (key == KEY_PRICE) ? "PRICE" : (key == KEY_STOCK) ? "STOCK" : "BRAND";
        cout << "\n=== INVENTORY SORTED BY " << by << (dir == 1 ? " (ASCENDING)" : " (DESCENDING)") << " ===\n";
        printTable(v, n);
        cout << "[Lab 8] " << algo << " used " << g_comparisons << " comparisons on " << n << " records\n";
        delete[] v;
    }
 
    // ---- main loop ---------------------------------------------------------
    void run() {
        while (true) {
            cout << "\n=================================================================\n"
                 << "  BRAND-AFFILIATED E-COMMERCE INVENTORY & SEARCH SYSTEM (CS201)\n"
                 << "=================================================================\n"
                 << "1. View Complete Catalog (Tabular View with Brand Affiliations)\n"
                 << "2. Filter Products by Official Brand (e.g., Apple, Samsung, Sony)\n"
                 << "3. Search Products by Keyword (Linear Search)\n"
                 << "4. Filter Products by Price Range (Binary Search)\n"
                 << "5. Add Item to Shopping Cart (Doubly Linked List Navigation)\n"
                 << "6. Process Customer Checkout Queue (FIFO Order Queue)\n"
                 << "7. Perform Admin Undo Action (LIFO Stack Pop)\n"
                 << "8. Display Partner Brand Banner Showcase (Circular List Cycle)\n"
                 << "9. Sort Inventory Display (By Price, Stock, or Brand)\n"
                 << "10. Exit\n"
                 << "=================================================================\n";
            int c = readInt("Enter your choice (1-10): ", 1, 10);
            if (c == 10) {
                cout << "\nThank you for using the system. Goodbye!\n";
                return;
            }
            switch (c) {
                case 1: viewCatalog();    break;
                case 2: filterByBrand();  break;
                case 3: searchMenu();     break;
                case 4: filterByPrice();  break;
                case 5: cartMenu();       break;
                case 6: checkoutMenu();   break;
                case 7: adminMenu();      break;
                case 8: brandShowcase();  break;
                case 9: sortInventory();  break;
            }
            if (c <= 4 || c == 9) pauseScreen();
        }
    }
};
 
// ============================================================================
int main() {
    Store store;
    store.seed();
    store.run();
    return 0;
}
 
