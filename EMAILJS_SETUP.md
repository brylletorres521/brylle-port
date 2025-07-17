# EmailJS Setup Guide

## What I've Implemented

I've added EmailJS to your portfolio contact form. The form will now actually send emails when someone submits it. Here's what was added:

1. **EmailJS CDN** - Added to your `index.html`
2. **EmailJS JavaScript** - Updated your `js/script.js` with EmailJS functionality
3. **Success/Error Messages** - Added CSS styles for better user feedback

## Setup Steps

### Step 1: Create EmailJS Account
1. Go to [EmailJS.com](https://www.emailjs.com/)
2. Sign up for a free account
3. Verify your email address

### Step 2: Create Email Service
1. In EmailJS dashboard, go to "Email Services"
2. Click "Add New Service"
3. Choose "Gmail" (or your preferred email provider)
4. Connect your email account (tbrylle7@gmail.com)
5. Note down the **Service ID** (e.g., `service_abc123`)

### Step 3: Create Email Template
1. Go to "Email Templates"
2. Click "Create New Template"
3. Use this template:

**Template Name:** `portfolio_contact`

**Subject:** `New Contact Message from {{from_name}}`

**Content:**
```
Hello Brylle,

You have received a new message from your portfolio website:

Name: {{from_name}}
Email: {{from_email}}
Subject: {{subject}}

Message:
{{message}}

Best regards,
Your Portfolio Website
```

4. Save the template and note down the **Template ID** (e.g., `template_xyz789`)

### Step 4: Get Your Public Key
1. Go to "Account" → "API Keys"
2. Copy your **Public Key** (e.g., `user_def456`)

### Step 5: Update Your Code
Replace the placeholder values in `js/script.js`:

```javascript
// Line 95: Replace YOUR_PUBLIC_KEY
emailjs.init("user_def456"); // Your actual public key

// Line 108: Replace YOUR_SERVICE_ID and YOUR_TEMPLATE_ID
emailjs.send('service_abc123', 'template_xyz789', {
    // ... rest of the code stays the same
})
```

## Final Configuration

Your `js/script.js` should look like this (with your actual values):

```javascript
// Initialize EmailJS
emailjs.init("user_def456"); // Your actual public key

// In the form submission:
emailjs.send('service_abc123', 'template_xyz789', {
    from_name: formValues.name,
    from_email: formValues.email,
    subject: formValues.subject,
    message: formValues.message,
    to_email: 'tbrylle7@gmail.com'
})
```

## Features Added

✅ **Real Email Sending** - Form actually sends emails to your inbox
✅ **Loading State** - Button shows "Sending..." with spinner
✅ **Success Message** - Green confirmation when email is sent
✅ **Error Handling** - Red error message if sending fails
✅ **Form Reset** - Form clears after successful submission
✅ **Responsive Design** - Works on all devices

## Testing

1. Fill out the contact form on your website
2. Submit the form
3. Check your email (tbrylle7@gmail.com) for the message
4. You should also see a success message on the website

## Free Tier Limits

EmailJS free tier includes:
- 200 emails per month
- Basic templates
- Gmail, Outlook, and other email services

This should be more than enough for a portfolio website!

## Troubleshooting

If emails aren't sending:
1. Check browser console for errors
2. Verify your EmailJS credentials are correct
3. Make sure your email service is properly connected
4. Check your spam folder

## Security Note

The public key is safe to use in frontend code. EmailJS handles the security on their servers. 