// 01-invoice-srp-ocp.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace std;

struct LineItem {
    string sku;
    int quantity{0};
    double unitPrice{0.0};
};
class InvoiceRenderer{
public:
    virtual string renderoutput(const vector<LineItem>& items, double subtotal ,double discount_total ,double tax , double grand)=0;
    virtual ~InvoiceRenderer() = default;

};
class PdfRenderer :public InvoiceRenderer{
    public:
    string renderoutput(const vector<LineItem>& items, double subtotal ,double discount_total, double tax , double grand){
        ostringstream pdf;
        pdf << "INVOICE\n";
        for (auto& it : items) {
            pdf << it.sku << " x" << it.quantity << " @ " << it.unitPrice << "\n";
        }
        pdf << "Subtotal: " << subtotal << "\n"
            << "Discounts: " << discount_total << "\n"
            << "Tax: " << tax << "\n"
            << "Total: " << grand << "\n";
        return pdf.str();       
    }
};
class InvoiceSharer {
public:
    virtual void share(const std::string& recipient, const std::string& content="") = 0;
    virtual ~InvoiceSharer() = default;
};
class EmailSharer:public InvoiceSharer{
    public:
    void share(const std::string& recipient, const std::string& content=""){
        if (!recipient.empty()) {
            cout << "[SMTP] Sending invoice to " << recipient << "...\n";
        }

    }
};
class Logger {
public:
    virtual void Log(const std::string& recipient, double grand) = 0;
    virtual ~Logger() = default;
};
class BasicLogger:public Logger{
    public:
    void Log(const std::string& recipient, double grand){
        cout << "[LOG] Invoice processed for " << recipient << " total=" << grand << "\n";
    }
};

class DiscountCalculator{
    public :
    static double Totaldiscount(const map<string, double>& discounts , double subtotal){
        double discount_total = 0.0;
        for (auto& kv : discounts) {
            const string& k = kv.first;
            double v = kv.second;
            if (k == "percent_off") {
                discount_total += subtotal * (v / 100.0);
            } else if (k == "flat_off") {
                discount_total += v;
            } else {
                // unknown ignored
            }
        }
        return discount_total;
    }

};
class InvoiceService {
    InvoiceRenderer * renderer;
    InvoiceSharer * sharer;
    Logger * logger;
public:
    void setRenderer(InvoiceRenderer * renderer){
        this->renderer= renderer;
    }
    void setSharer( InvoiceSharer * sharer){
       this->sharer=sharer;
    }
    void setLogger( Logger * logger){
       this->logger=logger;
    }
    double Calculatesubtotal(const vector<LineItem>& items){
        double subtotal = 0.0;
        for (auto& it : items) subtotal += it.unitPrice * it.quantity;
        return subtotal;
    }
    string process(const vector<LineItem>& items,
                   const map<string, double>& discounts,
                   const string& email) {
        // pricing
        double subtotal = Calculatesubtotal(items);

        // discounts (tightly coupled)
        double discount_total = DiscountCalculator::Totaldiscount(discounts,subtotal);
       
        // tax inline
        double tax = (subtotal - discount_total) * 0.18;
        double grand = subtotal - discount_total + tax;

        // rendering inline (pretend PDF)
        string output = renderer->renderoutput(items,subtotal,discount_total, tax, grand);
     
        // email I/O inline (tight coupling)
        sharer->share(email);


        // logging inline
        logger->Log(email,grand);
        
        return output;
    }

};
class Tester {
    InvoiceService *svc;
    
public :
    Tester(InvoiceService * svc){
        this->svc=svc;
    }
    double computeTotal(const vector<LineItem>& items,
                        const map<string, double>& discounts) {
        string dummyEmail = "noreply@example.com";
        auto rendered = svc->process(items, discounts, dummyEmail);
        auto pos = rendered.rfind("Total:");
        if (pos == string::npos) throw runtime_error("No total");
        auto line = rendered.substr(pos + 6);
        return stod(line);
    }


};


int main() {
    InvoiceService svc; 
    
    InvoiceRenderer *render1= new PdfRenderer;
    svc.setRenderer(render1);
    InvoiceSharer *share1 = new EmailSharer;
    svc.setSharer(share1);
    Logger *logger1 = new BasicLogger;
    svc.setLogger(logger1);
    // Create items
    vector<LineItem> items = { {"ITEM-001", 3, 100.0}, {"ITEM-002", 1, 250.0} };
    map<string,double> discounts = { {"percent_off", 10.0} };
    cout << svc.process(items, discounts, "customer@example.com") << endl;
    delete render1;
    delete share1;
    delete logger1;
    return 0;
}
