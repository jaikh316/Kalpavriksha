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
#define MIN_TICKETS_FOR_DISCOUNT 5
typedef enum {
    STANDARD = 1,
    PREMIUM,
    RECLINER
} TicketCategory;

typedef struct {
    char customerName[NAME_SIZE];
    int noOfTickets;
    TicketCategory category;
    double subtotal;
    double discount;
    double finalAmount;
} BookingTicket;
int inputDetails(char *customerName, int *noOfTickets, TicketCategory *category) {
  int categoryChoice;
  if(scanf("%49s", customerName) != 1) {
      printf("Invalid name!\n");
      return 0;
  }
  while(1) {
    if(scanf("%d", noOfTickets) != 1) {
      printf("Invalid number of tickets!\n");
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
      continue;
    }
    if(*noOfTickets < MIN_TICKETS || *noOfTickets > MAX_TICKETS) {
        printf("Invalid number of tickets! Enter number between %d - %d\n", MIN_TICKETS, MAX_TICKETS);
        continue;
    }
    break;
  }
  while(1) {
    if(scanf("%d", &categoryChoice) !=1) {
      printf("Invalid ticket category!\n");
      int ch;
      while((ch = getchar()) != '\n' && ch != EOF) {
      }
      continue;
    }
    if(categoryChoice < MIN_CATEGORY_CHOICE || categoryChoice > MAX_CATEGORY_CHOICE) {
      printf("Invalid ticket category! Enter number between %d - %d\n", MIN_CATEGORY_CHOICE, MAX_CATEGORY_CHOICE);
      continue;
    }
    break;
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
  if(noOfTickets >= MIN_TICKETS_FOR_DISCOUNT) {
    discount = subtotal * DISCOUNT_RATE;
  }
  return discount;
}
double finalBill(double subtotal, double discount) {
  return subtotal - discount;
}
void displayBill(const BookingTicket *booking) {
  printf("Customer: %s\n", booking->customerName);
  printf("Ticket Category: %s\n", selectCategory(booking->category));
  printf("Tickets: %d\n", booking->noOfTickets);
  printf("Subtotal: %.2f\n", booking->subtotal);
  printf("Discount: %.2f\n", booking->discount);
  printf("Final Bill: %.2f\n", booking->finalAmount);
}
int main() {
  BookingTicket booking;
  if(!inputDetails(booking.customerName, &booking.noOfTickets, &booking.category)) {
    return 1;
  }
  booking.subtotal = calculateSubtotal(booking.noOfTickets, booking.category);
  booking.discount = applyDiscount(booking.subtotal, booking.noOfTickets);
  booking.finalAmount = finalBill(booking.subtotal, booking.discount);
  displayBill(&booking);
  return 0;
}