#include <stdio.h>
#define NAME_SIZE 50
#define MAX_TICKETS 20
#define MIN_TICKETS 1
#define MAX_CATEGORY_CHOICE 3
#define MIN_CATEGORY_CHOICE 1
#define STANDARD_PRICE 150
#define PREMIUM_PRICE 250
#define RECLINER_PRICE 400
#define DISCOUNT_RATE 0.10
typedef enum {
    STANDARD = 1,
    PREMIUM,
    RECLINER
} TicketCategory;
int inputDetails(char *customerName, int *noOfTickets, TicketCategory *category) {
  int categoryChoice;
  if(scanf("%49s", customerName) != 1) {
      printf("Invalid name!\n");
      return 0;
  }
  if(scanf("%d", noOfTickets) != 1) {
    printf("Invalid number of tickets!\n");
    return 0;
  }
  if(*noOfTickets < MIN_TICKETS || *noOfTickets > MAX_TICKETS) {
      printf("Invalid number of tickets! Enter number between %d - %d\n", MIN_TICKETS, MAX_TICKETS);
      return 0;
  }
  if(scanf("%d", &categoryChoice) !=1) {
    printf("Invalid ticket category!\n");
    return 0;
  }
  if(categoryChoice < MIN_CATEGORY_CHOICE || categoryChoice > MAX_CATEGORY_CHOICE) {
    printf("Invalid ticket category! Enter number between %d - %d\n", MIN_CATEGORY_CHOICE, MAX_CATEGORY_CHOICE);
    return 0;
  }
  *category =(TicketCategory)categoryChoice;
  return 1;
}
const char *selectCategory(TicketCategory category) {
    const char *categoryNames[] = {"Standard", "Premium", "Recliner"};
    if(category < STANDARD || category > RECLINER) {
        return "Invalid";
    }
    return categoryNames[category - STANDARD];
}
double calculateSubtotal(int noOfTickets, TicketCategory category) {
  int ticketPrice;
  switch(category) {
    case STANDARD : ticketPrice = STANDARD_PRICE;
                    break;
    case PREMIUM : ticketPrice = PREMIUM_PRICE;
                    break;
    case RECLINER : ticketPrice = RECLINER_PRICE;
                    break;
    default : return 0.0;
  }
  return (double)noOfTickets * ticketPrice;
}
double applyDiscount(double subtotal, int noOfTickets) {
  double discount = 0.0;
  if(noOfTickets >= 5) {
    discount = subtotal * DISCOUNT_RATE;
  }
  return discount;
}
double finalBill(double subtotal, double discount) {
  return subtotal - discount;
}
void displayBill(const char *customerName, int noOfTickets, const char *categoryName, double subtotal, double discount, double finalAmount) {
  printf("Customer: %s\n", customerName);
  printf("Ticket Category: %s\n", categoryName);
  printf("Tickets: %d\n", noOfTickets);
  printf("Subtotal: %.2f\n", subtotal);
  printf("Discount: %.2f\n", discount);
  printf("Final Bill: %.2f\n", finalAmount);
}
int main() {
  char customerName[NAME_SIZE];
  int noOfTickets;
  TicketCategory category;
  if(!inputDetails(customerName, &noOfTickets, &category)) {
    return 1;
  }
  const char *categoryName = selectCategory(category);
  double subtotal = calculateSubtotal(noOfTickets, category);
  double discount = applyDiscount(subtotal, noOfTickets);
  double finalAmount = finalBill(subtotal, discount);
  displayBill(customerName, noOfTickets, categoryName, subtotal, discount, finalAmount);
  return 0;
}